//#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QtWebSockets/QWebSocket>
#include "Connection.h"
#include "Request.h"

#define TAG     "eg::rest::Connection"

TM_CORE_BEGIN(rest)

//
Connection::Connection()
{
    _serverRoot="";
    _account=nullptr;
    _networkManager=new QNetworkAccessManager();
}

//
Connection::~Connection()
{
    reset();
}

//
QString                     Connection::apiRoot(int version)
{
    return(_serverRoot+QString("/v%1/api/").arg(version));
}

//
QString                     Connection::wsRoot(int version)
{
    //return("ws://127.0.0.1:3123");
    QString s=_serverRoot+QString("/v%1/wsapi").arg(version);
    if(s.startsWith("http://"))
    {
        return(s.replace("http://","ws://"));
    }
    if(s.startsWith("https://"))
    {
        return(s.replace("https://","wss://"));
    }
    return("");
}

//
Connection::AuthResult      Connection::openSession(const QString& serverRoot,const QString& agent,const QString& previousToken)
{
    _serverRoot=serverRoot;
    QUrl url(apiRoot()+"session/open");
    QNetworkRequest* req=new QNetworkRequest();
    req->setUrl(url);
    req->setRawHeader("Content-Type", "application/json");
    QJsonObject         postData;
    postData.insert("agent",agent);
    postData.insert("token",previousToken);
    postData.insert("version",QJsonValue::fromVariant(version));
    QNetworkReply*reply=_networkManager->post(*req,QJsonDocument(postData).toJson());
    AuthResult authResult=QSslSocket::supportsSsl()?AuthResult::NetworkError:AuthResult::SslLibraryError;
    if(1)
    {
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        if (reply->error() == QNetworkReply::NoError)
        {
            TM_TRACE(TAG, "Authentification response received");
            QByteArray rawData=reply->readAll();
            QString data = QString::fromUtf8(rawData);
            //EG_TRACE(TAG, data.toStdString().c_str());

            QJsonDocument document=QJsonDocument::fromJson(rawData);
            QJsonObject object=document.object();
            _token=object["token"].toString();
            if(_token.length())
            {
                TM_INFO(TAG, "Session openned");
                _token          =object["token"].toString();
                _humanity       =object["humanity"].toInt();
                _sessionKey     =QByteArray::fromHex(object["key"].toString().toLatin1());
                authResult      =AuthResult::Succeeded;
                _connectedSince =QDateTime::currentDateTime();
            }
            else
            {
                if(object["version"].toVariant().toUInt()==rest::version)
                {
                    TM_ERROR(TAG, "Failed to open session");
                    authResult      =AuthResult::OpenSessionFailed;
                }
                else
                {
                    TM_ERROR(TAG, "API version is different from server");
                    authResult      =AuthResult::APIVersionMismatch;
                }
            }
        }
        else
        {
            TM_ERROR(TAG, "Network error");
        }
        delete(req);
    }
    return(authResult);
}

//
Connection::AuthResult      Connection::authenticate(const QString& username,const QString& password)
{
    QUrl            url(apiRoot()+"session/authenticate");
    QJsonObject     postData;
    postData.insert("email",username);
    postData.insert("password",password);
    QNetworkRequest* req=new QNetworkRequest();
    req->setUrl(url);
    QString bearer=QString("Bearer ")+_token;
    req->setRawHeader("authorization", bearer.toStdString().c_str());
    req->setRawHeader("Content-Type", "application/json");
    QNetworkReply*reply=_networkManager->post(*req,QJsonDocument(postData).toJson());
    AuthResult authResult=QSslSocket::supportsSsl()?AuthResult::NetworkError:AuthResult::SslLibraryError;
    if(1)
    {
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        if (reply->error() == QNetworkReply::NoError)
        {
            TM_TRACE(TAG, "Authentification response received");
            QByteArray rawData=reply->readAll();
            QString data = QString::fromUtf8(rawData);
            QJsonDocument document=QJsonDocument::fromJson(rawData);
            QJsonObject object=document.object();
            _humanity   =object["humanity"].toInt();
            if(object["authenticated"].toBool())
            {
                TM_INFO(TAG, "Authentification succeeded");
                authResult      =loadAccount()?AuthResult::Succeeded:AuthResult::LoadAccountFailed;
                _connectedSince =QDateTime::currentDateTime();
            }
            else
            {
                TM_ERROR(TAG, "Authentification failed");
                authResult=AuthResult::AuthFailed;
            }
        }
        else
        {
            TM_ERROR(TAG, "Network error");
        }
        delete(req);
    }
    return(authResult);
}

//
Connection::AuthResult  Connection::createAccount(const QString& username,const QString& password)
{
    QJsonObject     account;
    account.insert("email",username);
    account.insert("password",password);
    tmio::rest::RequestResult result=rPOST("session/createaccount",account);
    if(result._statusCode==200)
    {
        // yes account created, build UI login
        _humanity=result._document.object()["humanity"].toInt();
        if(result._document.object()["created"].toBool())
        {
            return(AuthResult::Succeeded);
        }
        else
        {
            if(result._document.object()["busy"].toBool())
            {
                return(AuthResult::TooManyAccounts);
            }
            return(AuthResult::CreateAccountFailed);
        }
    }
    return(AuthResult::NetworkError);
}

//
Connection::AuthResult  Connection::recoverAccount(const QString& username,const QString& password)
{
    QJsonObject     account;
    account.insert("email",username);
    account.insert("password",password);
    tmio::rest::RequestResult result=rPOST("session/recoveraccount",account);
    if(result._statusCode==200)
    {
        // account created
        _humanity=result._document.object()["humanity"].toInt();
        if(result._document.object()["recovered"].toBool())
        {
            return(AuthResult::Succeeded);
        }
        else
        {
            return(AuthResult::RecoverAccountFailed);
        }
    }
    return(AuthResult::NetworkError);
}

//
QNetworkAccessManager*  Connection::networkManager()
{
    return(_networkManager);
}

//
Request*                Connection::createRequest(QString url,int version)
{
    Request*    req=new Request(*this,url,version);
    return(req);
}

//
RequestResult           Connection::rGET(QString url)
{
    Request*      request=createRequest(url);
    RequestResult result=request->get(true);
    request->deleteLater();
    return(result);
}

//
RequestResult           Connection::rDELETERESOURCE(QString url)
{
    Request*      request=createRequest(url);
    RequestResult result=request->deleteResource(true);
    request->deleteLater();
    return(result);
}

//
RequestResult           Connection::rPOST(QString url,QByteArray postData)
{
    Request*      request=createRequest(url);
    RequestResult result=request->post(postData,true);
    request->deleteLater();
    return(result);
}

//
RequestResult           Connection::rPOST(QString url,QJsonObject postData)
{
    Request*      request=createRequest(url);
    RequestResult result=request->post(postData,true);
    request->deleteLater();
    return(result);
}

//
QString                 Connection::token()
{
    return(_token);
}

//
QByteArray              Connection::sessionKey()
{
    return(_sessionKey);
}

//
QWebSocket*             Connection::createWebSocket()
{
    QWebSocket*         socket=new QWebSocket();
    connect(socket,&QWebSocket::connected,this,&Connection::onWsconnected);
    connect(socket,&QWebSocket::binaryFrameReceived,this,&Connection::onWsBinaryFrameReceived);
    connect(socket,&QWebSocket::textFrameReceived,this,&Connection::onWsTextFrameReceived);
    connect(socket,&QWebSocket::bytesWritten,this,&Connection::onWsBytesWritten);
    return(socket);
}

//
QDateTime               Connection::connectedSince()
{
    return(_connectedSince);
}

//
void                    Connection::stats(qint64& calls,qint64& bytesIn,qint64& bytesOut)
{
    calls       =_calls;
    bytesIn     =_bytesIn;
    bytesOut    =_bytesOut;
}

//
void                    Connection::wsStats(qint64& connections,qint64& bytesIn,qint64& bytesOut)
{
    connections =_wsConnections;
    bytesIn     =_wsBytesIn;
    bytesOut    =_wsBytesOut;
}

//
void                    Connection::resetStats()
{
    _connectedSince =QDateTime::currentDateTime();
    _calls          =0;
    _bytesIn        =0;
    _bytesOut       =0;
    _wsConnections  =0;
    _wsBytesIn      =0;
    _wsBytesOut     =0;
}

//
bool                    Connection::isHuman()
{
    return(_humanity>=100);
}

//
api::Account*           Connection::account()
{
    return(_account);
}

//
void                    Connection::reset()
{
    TM_TRACE(TAG,"Reset connexion");
    delete(_account);
}

//
bool                    Connection::loadAccount()
{
    /* Account */
    TM_TRACE(TAG,"Loading account");
    tmio::rest::RequestResult accountResult  =rGET("accounts/current");
    if(accountResult._statusCode==200)
    {
        QJsonObject account=accountResult._document.object();
        _account=new tmio::api::Account(this,account);
        _account->loadAvatar();
        _account->loadProperties();
        return(true);
    }
    return(false);
}

//
void                    Connection::onWsconnected()
{
    _wsConnections++;
}

//
void                    Connection::onWsBinaryFrameReceived(const QByteArray &frame, bool /*isLastFrame*/)
{
    _wsBytesIn+=frame.length();
}

//
void                    Connection::onWsBinaryMessageReceived(const QByteArray &message)
{
    _wsBytesIn+=message.length();
}

//
void                    Connection::onWsTextFrameReceived(const QString &frame, bool /*isLastFrame*/)
{
    _wsBytesIn+=frame.length();
}

//
void                    Connection::onWsTextMessageReceived(const QString &message)
{
    _wsBytesIn+=message.length();
}

//
void                    Connection::onWsBytesWritten(qint64 bytes)
{
    _wsBytesOut+=bytes;
}


TM_CORE_END()

