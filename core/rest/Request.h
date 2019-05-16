#pragma once
#include "rest.h"
#include "RequestResult.h"
#include "Connection.h"
#include <QObject>
#include <QJsonObject>
TM_CORE_BEGIN(rest)


class Request : public QObject
{
    Q_OBJECT
public:
    Request(Connection&,QString url,int apiVersion=defaultRequestVersion);
    virtual ~Request();
    void                    setUrl(QString url);
    RequestResult           get(bool blocking);
    RequestResult           deleteResource(bool blocking);
    RequestResult           post(QByteArray postData,const QString& contentType,bool blocking);
    RequestResult           post(QByteArray postData,bool blocking);
    RequestResult           post(QJsonObject postData,bool blocking);

private:
    Connection&             _conn;
    QNetworkRequest*        _req;
    QNetworkReply*          _reply;
    bool                    _canceled;
    QString                 _url;

public slots:
    void                    onNetworkRequestFinished();

signals:
    void                    finished(tmio::rest::RequestResult&);

private:
    int                     _apiVersion;
};

TM_CORE_END()
