#pragma once
#include "rest.h"
#include "Connection.h"
#include "RequestResult.h"
#include "../api/Account.h"
#include <QVector>
#include <QtNetwork/QNetworkAccessManager>

class QWebSocket;

TM_CORE_BEGIN(rest)

class Request;


class Connection : public QObject
{
    Q_OBJECT
public:
    enum AuthResult
    {
        Succeeded,
        NetworkError,
        SslLibraryError,
        OpenSessionFailed,
        AuthFailed,
        APIVersionMismatch,
        LoadAccountFailed,
        CreateAccountFailed,
        TooManyAccounts,
        RecoverAccountFailed
    };

    Connection();
    virtual ~Connection();

    QString                 apiRoot(int version=defaultRequestVersion);
    QString                 wsRoot(int version=defaultRequestVersion);
    AuthResult              openSession(const QString& serverRoot,const QString& agent,const QString& previousToken);
    AuthResult              authenticate(const QString& username,const QString& password);
    AuthResult              createAccount(const QString& username,const QString& password);
    AuthResult              recoverAccount(const QString& username,const QString& password);
    QNetworkAccessManager*  networkManager();
    Request*                createRequest(QString url,int version=defaultRequestVersion);
    RequestResult           rGET(QString url);
    RequestResult           rPOST(QString url,QByteArray postData);
    RequestResult           rPOST(QString url,QJsonObject postData);
    RequestResult           rDELETERESOURCE(QString url);
    QString                 token();
    QByteArray              sessionKey();
    QWebSocket*             createWebSocket();
    QDateTime               connectedSince();
    void                    stats(qint64& calls,qint64& bytesIn,qint64& bytesOut);
    void                    wsStats(qint64& connections,qint64& bytesIn,qint64& bytesOut);
    void                    resetStats();
    bool                    isHuman();

    /* Account */
    api::Account*           account();

private:
    void                    reset();
    bool                    loadAccount();

public slots:
    void                    onWsconnected();
    void                    onWsBinaryFrameReceived(const QByteArray &frame, bool isLastFrame);
    void                    onWsBinaryMessageReceived(const QByteArray &message);
    void                    onWsTextFrameReceived(const QString &frame, bool isLastFrame);
    void                    onWsTextMessageReceived(const QString &message);
    void                    onWsBytesWritten(qint64 bytes);

signals:

private:
    QNetworkAccessManager*  _networkManager;
    QString                 _token;
    QByteArray              _sessionKey;
    QString                 _serverRoot;
    api::Account*           _account;
    QDateTime               _connectedSince;
    qint64                  _calls=0;
    qint64                  _bytesIn=0;
    qint64                  _bytesOut=0;
    qint64                  _wsConnections=0;
    qint64                  _wsBytesIn=0;
    qint64                  _wsBytesOut=0;
    int                     _humanity=0;
    friend class            Request;
};

TM_CORE_END()
