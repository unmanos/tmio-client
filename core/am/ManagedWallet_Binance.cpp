#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QtNetwork/QNetworkReply>
#include "ManagedWallet_Binance.h"

//
#define    TAG          "tmio::am::ManagedWallet_Binance"

//
#define    API_URL      "https://api.binance.com/api"
#define    ACCOUNT_V3   "/v3/account"

TM_CORE_BEGIN(am)

//
ManagedWallet_Binance::ManagedWallet_Binance(AssetsManager* assetsManager,QJsonObject config) : ManagedWallet(assetsManager,config)
{
    _apiKey     =config["exchange"].toObject()["binance"].toObject()["apiKey"].toString();
    _apiSecret  =config["exchange"].toObject()["binance"].toObject()["apiSecret"].toString();
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&ManagedWallet_Binance::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
QByteArray                      ManagedWallet_Binance::getHMAC(const QString & message)
{
    QUrl url(message);
    return QMessageAuthenticationCode::hash(url.toEncoded(), _apiSecret.toLocal8Bit(), QCryptographicHash::Sha256).toHex();
}

//
void                            ManagedWallet_Binance::getAccount()
{
    QString surl(QString(API_URL) + QString(ACCOUNT_V3) + QString("?"));
    QString query(QString("timestamp=") + QString::number(QDateTime::currentMSecsSinceEpoch()) + QString("&recvWindow=5000"));
    QString signature = QString(getHMAC(query));
    query += QString("&signature=") + signature;
    QUrl url(surl + query);

    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netReq.setRawHeader("X-MBX-APIKEY", _apiKey.toLocal8Bit());

    QNetworkReply*      reply=networkManager()->get(netReq);
    connect(reply, &QNetworkReply::finished, this, &ManagedWallet_Binance::onReceiveAccount);
}

//
void                            ManagedWallet_Binance::onUpdate()
{
    TM_TRACE(TAG,"onUpdate");
    getAccount();
}

//
void                            ManagedWallet_Binance::onReceiveAccount()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            if(doc.object().contains("balances"))
            {
                QJsonArray  balances=doc.object()["balances"].toArray();
                resetAssets();
                for(int i=0;i<balances.count();i++)
                {
                    QJsonObject     balance=balances.at(i).toObject();
                    QString         asset=balance["asset"].toString();
                    double          free=balance["free"].toString().toDouble();
                    double          locked=balance["locked"].toString().toDouble();
                    double          total=free+locked;
                    if(total!=0)
                    {
                        // get cmcid from asset
                        QString assetID=marketData()->loopupExchangeAsset(md::exchangeBinance,asset);
                        //TM_TRACE(TAG,QString("%1 (%2): %3").arg(asset).arg(coinID).arg(QString::number(total,'f', 8)));
                        setAsset(asset,assetID,total);
                    }
                }
            }
            TM_TRACE(TAG,raw.toStdString().c_str());
            qDebug() << raw.toStdString().c_str();
        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed account (%1)").arg(statusCode));
    }
    delete(reply);
}


TM_CORE_END()
