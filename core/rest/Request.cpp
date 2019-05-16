#include "Request.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>

#define TAG         "eg::rest::Request"

TM_CORE_BEGIN(rest)

//
Request::Request(Connection& conn,QString url,int apiVersion) : _conn(conn), _apiVersion(apiVersion)
{
    QString apiRoot=conn.apiRoot(_apiVersion);
    QString fullUrl=apiRoot+url;
    _req=new QNetworkRequest(QUrl(fullUrl));
    _reply      =nullptr;
    _canceled   =false;
    _url=fullUrl;

    QString bearer=QString("Bearer ")+conn.token();
    _req->setRawHeader("authorization", bearer.toStdString().c_str());
}

//
Request::~Request()
{
    delete _req;
    delete _reply;
}

//
void            Request::setUrl(QString url)
{
    _canceled   =true;
    QString fullUrl=_conn.apiRoot(_apiVersion)+url;
    _req->setUrl(fullUrl);
}

//
RequestResult   Request::get(bool blocking)
{
    RequestResult result(this);
    _canceled=false;
    if(_reply)
    {
        _reply->deleteLater();
    }
    _reply=_conn.networkManager()->get(*_req);
    _conn._bytesOut+=_url.length();
    _conn._calls++;
    if(!blocking)
    {
        connect(_reply, SIGNAL(finished()), this, SLOT(onNetworkRequestFinished()));
        return(RequestResult(this));
    }
    // blocking request
    QEventLoop loop;
    connect(_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();
    if( (_reply->error() == QNetworkReply::NoError)&&(!_canceled) )
    {
        result._statusCode=_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result._raw=_reply->readAll();
        result._document=QJsonDocument::fromJson(result._raw);
    }
    else
    {
        TM_ERROR(TAG,QString("Request failed (%1)").arg(_url));
    }
    return(result);
}

//
RequestResult   Request::deleteResource(bool blocking)
{
    RequestResult result(this);
    _canceled=false;
    if(_reply)
    {
        _reply->deleteLater();
    }
    _reply=_conn.networkManager()->deleteResource(*_req);
    _conn._bytesOut+=_url.length();
    _conn._calls++;
    if(!blocking)
    {
        connect(_reply, SIGNAL(finished()), this, SLOT(onNetworkRequestFinished()));
        return(RequestResult(this));
    }
    // blocking request
    QEventLoop loop;
    connect(_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();
    if( (_reply->error() == QNetworkReply::NoError)&&(!_canceled) )
    {
        result._statusCode=_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result._raw=_reply->readAll();
        result._document=QJsonDocument::fromJson(result._raw);
    }
    else
    {
        TM_ERROR(TAG,QString("Request failed (%1)").arg(_url));
    }
    return(result);
}

//
RequestResult   Request::post(QByteArray postData,const QString& contentType,bool blocking)
{
    RequestResult result(this);
    _req->setRawHeader("Content-Type", contentType.toUtf8());
    _canceled=false;
    if(_reply)
    {
        _reply->deleteLater();
    }
    _reply=_conn.networkManager()->post(*_req,postData);
    _conn._bytesOut+=_url.length();
    _conn._bytesOut+=postData.length();
    _conn._calls++;
    if(!blocking)
    {
        connect(_reply, SIGNAL(finished()), this, SLOT(onNetworkRequestFinished()));
        return(RequestResult(this));
    }
    // blocking request
    QEventLoop loop;
    connect(_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();
    TM_TRACE(TAG,"POST "+_req->url().toString());
    if( (_reply->error() == QNetworkReply::NoError)&&(!_canceled) )
    {
        result._statusCode=_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result._raw=_reply->readAll();
        result._document=QJsonDocument::fromJson(result._raw);
        TM_TRACE(TAG,"Request succeeded");
    }
    else
    {
        TM_ERROR(TAG,QString("Request failed (%1)").arg(_url));
    }
    return(result);
}

//
RequestResult   Request::post(QByteArray postData,bool blocking)
{
    return(post(postData,"application/octet-stream",blocking));
}

//
RequestResult   Request::post(QJsonObject postData,bool blocking)
{
    return(post(QJsonDocument(postData).toJson(),"application/json",blocking));
}

//
void            Request::onNetworkRequestFinished()
{
    QNetworkReply* reply=_reply;
    RequestResult r(this);
    if( (reply->error() == QNetworkReply::NoError)&&(!_canceled) )
    {
        r._statusCode=_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray rawData=reply->readAll();
        _conn._bytesIn+=rawData.length();
        r._raw=rawData;
        r._document=QJsonDocument::fromJson(rawData);
    }
    else
    {
        TM_ERROR(TAG,QString("Request failed (%1)").arg(_url));
    }
    emit finished(r);
}

TM_CORE_END()

