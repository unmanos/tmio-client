#include "PrivateWallet_Blockchain.h"

#define TAG                 "PrivateWallet_Blockchain"
#define API_BALANCE         "https://blockchain.info/balance?active="

//
TM_CORE_BEGIN(am)

//
PrivateWallet_Blockchain::PrivateWallet_Blockchain(AssetsManager* assetsManager,QJsonObject config) : PrivateWallet(assetsManager,config)
{
    QString         address=config["private"].toObject()["bitcoin"].toObject()["address"].toString();
    foreach(QString addr,address.split("|"))
    {
        _addresses[addr]=QJsonObject();
    }
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&PrivateWallet_Blockchain::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
void                    PrivateWallet_Blockchain::onUpdate()
{
    QString         queryAddresses;
    foreach(QString addr, _addresses.keys())
    {
        queryAddresses+=queryAddresses.length()?"|":"";
        queryAddresses+=addr;
    }

    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(API_BALANCE+queryAddresses);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply*      reply=networkManager()->get(netReq);
    connect(reply, &QNetworkReply::finished, this, &PrivateWallet_Blockchain::onReceive);
}

//
void                    PrivateWallet_Blockchain::onReceive()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            resetAssets();
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            qint64          satoshis=0;
            foreach(QString addr,doc.object().keys())
            {
                QJsonObject     addrResult=doc.object()[addr].toObject();
                _addresses[addr]=addrResult;
                satoshis+=addrResult["final_balance"].toVariant().toLongLong();
            }
            double          amount=static_cast<double>(satoshis)/100000000;
            setAsset("BTC","bitcoin",amount);
        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed account (%1)").arg(statusCode));
    }
    delete(reply);
}

TM_CORE_END()
