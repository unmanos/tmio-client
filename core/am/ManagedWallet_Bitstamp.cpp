#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QMessageAuthenticationCode>
#include "ManagedWallet_Bitstamp.h"

//
#define    TAG          "tmio::am::ManagedWallet_Bitstamp"

//
#define    API_URL      "https://www.bitstamp.net/api"
#define    BALANCE_V2   "/v2/balance/"

//
TM_CORE_BEGIN(am)

//
ManagedWallet_Bitstamp::ManagedWallet_Bitstamp(AssetsManager* assetsManager,QJsonObject config) : ManagedWallet(assetsManager,config)
{
    _customerID =config["exchange"].toObject()["bitstamp"].toObject()["customerID"].toString();
    _apiKey     =config["exchange"].toObject()["bitstamp"].toObject()["apiKey"].toString();
    _apiSecret  =config["exchange"].toObject()["bitstamp"].toObject()["apiSecret"].toString();
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&ManagedWallet_Bitstamp::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
QByteArray                      ManagedWallet_Bitstamp::getHMAC(const QString & message)
{
    QUrl url(message);
    return QMessageAuthenticationCode::hash(url.toEncoded(), _apiSecret.toLocal8Bit(), QCryptographicHash::Sha256).toHex();
}

//
void                            ManagedWallet_Bitstamp::getAccount()
{
    QString url(QString(API_URL) + QString(BALANCE_V2));
    QString nonce=QString::number(QDateTime::currentMSecsSinceEpoch());
    QString signature = QString(getHMAC(nonce+_customerID+_apiKey)).toUpper();

    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("key", _apiKey);
    postData.addQueryItem("signature", signature);
    postData.addQueryItem("nonce", nonce);

    QNetworkReply*  reply=networkManager()->post(netReq,postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &ManagedWallet_Bitstamp::onReceiveBalance);
}

//
void                            ManagedWallet_Bitstamp::onUpdate()
{
    TM_TRACE(TAG,"onUpdate");
    getAccount();
}

//
void                            ManagedWallet_Bitstamp::onReceiveBalance()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        TM_TRACE(TAG,"Received account");
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            if(doc.isObject())
            {
                resetAssets();
                QStringList     keys=doc.object().keys();
                foreach(QString key, keys)
                {
                    double balance=doc.object()[key].toString().toDouble();
                    if(key.endsWith("_balance"))
                    {
                        QStringList parts=key.toUpper().split('_',QString::SkipEmptyParts);
                        QString symbol=parts[0];
                        TM_TRACE(TAG,QString("%1: %2").arg(symbol).arg(balance));
                        if(balance!=0)
                        {
                            QString assetID=marketData()->loopupExchangeAsset(md::exchangeBitstamp,symbol);
                            setAsset(symbol,assetID,balance);
                        }
                    }
                }
            }
            TM_TRACE(TAG,raw.toStdString().c_str());
        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed account (%1)").arg(statusCode));
    }
    delete(reply);
}


TM_CORE_END()
