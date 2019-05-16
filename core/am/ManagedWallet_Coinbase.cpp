#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QtNetwork/QNetworkReply>
#include "ManagedWallet_Coinbase.h"

//
#define    TAG          "tmio::am::ManagedWallet_Coinbase"

//
#define    API_URL      "https://api.coinbase.com"
#define    API_ACCOUNT  "/v2/accounts"


TM_CORE_BEGIN(am)

//
ManagedWallet_Coinbase::ManagedWallet_Coinbase(AssetsManager* assetsManager,QJsonObject config) : ManagedWallet(assetsManager,config)
{
    _apiKey     =config["exchange"].toObject()["coinbase"].toObject()["apiKey"].toString();
    _apiSecret  =config["exchange"].toObject()["coinbase"].toObject()["apiSecret"].toString();
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&ManagedWallet_Coinbase::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
QByteArray                      ManagedWallet_Coinbase::getHMAC(const QString & message)
{
    QUrl url(message);
    return QMessageAuthenticationCode::hash(url.toEncoded(), _apiSecret.toLocal8Bit(), QCryptographicHash::Sha256).toHex();
}

//
void                            ManagedWallet_Coinbase::getAccount()
{
    QString url(QString(API_ACCOUNT));// + QString("?"));
    QString timestamp=QString::number(QDateTime::currentSecsSinceEpoch());
    QString message(timestamp+"GET"+url);
    QString signature = QString(getHMAC(message));

    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(API_URL+url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netReq.setRawHeader("CB-ACCESS-KEY", _apiKey.toLocal8Bit());
    netReq.setRawHeader("CB-ACCESS-SIGN", signature.toLocal8Bit());
    netReq.setRawHeader("CB-ACCESS-TIMESTAMP", timestamp.toLocal8Bit());

    QNetworkReply*      reply=networkManager()->get(netReq);
    connect(reply, &QNetworkReply::finished, this, &ManagedWallet_Coinbase::onReceiveAccount);
}

//
void                            ManagedWallet_Coinbase::onUpdate()
{
    TM_TRACE(TAG,"onUpdate");
    getAccount();
}

//
void                            ManagedWallet_Coinbase::onReceiveAccount()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(reply->error() == QNetworkReply::NoError)
    {
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            if(doc.object().contains("data"))
            {
                QJsonArray  datas=doc.object()["data"].toArray();
                resetAssets();
                for(int i=0;i<datas.count();i++)
                {
                    QJsonObject     data=datas.at(i).toObject();
                    QJsonObject     balance=data["balance"].toObject();
                    if(!balance.isEmpty())
                    {
                        QString     currency=balance["currency"].toString();
                        double      amount  =balance["amount"].toString().toDouble();
                        if(amount!=0)
                        {
                            QString assetID=marketData()->loopupExchangeAsset(md::exchangeCoinbase,currency);
                            setAsset(currency,assetID,amount);
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
