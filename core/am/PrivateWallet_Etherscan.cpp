#include "PrivateWallet_Etherscan.h"

#define TAG                 "PrivateWallet_Blockchain"

#define API_BALANCE         "https://api.etherscan.io/api?module=account&action=balancemulti&address="


//
TM_CORE_BEGIN(am)

//
PrivateWallet_Etherscan::PrivateWallet_Etherscan(AssetsManager* assetsManager,QJsonObject config) : PrivateWallet(assetsManager,config)
{
    QString         address=config["private"].toObject()["ethereum"].toObject()["address"].toString();
    foreach(QString addr,address.split("|"))
    {
        _addresses[addr]=QJsonObject();
    }
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&PrivateWallet_Etherscan::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
void                    PrivateWallet_Etherscan::onUpdate()
{
    // https://blockchain.info/balance?active=1CuafYtHT3VEtjBSRcoC8uDqHZUahbeajL|1FTRKKqCq8iGvNr3FU5vcR9GhPm2vCC59V
    QString         queryAddresses;
    foreach(QString addr, _addresses.keys())
    {
        queryAddresses+=queryAddresses.length()?",":"";
        queryAddresses+=addr;
    }

    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(API_BALANCE+queryAddresses);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply*      reply=networkManager()->get(netReq);
    connect(reply, &QNetworkReply::finished, this, &PrivateWallet_Etherscan::onReceive);
}

//
void                    PrivateWallet_Etherscan::onReceive()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        //TM_TRACE(TAG,"Received account");
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            resetAssets();
            QByteArray      raw     =reply->readAll();
            QJsonDocument   doc     =QJsonDocument::fromJson(raw);
            QJsonArray      result  =doc.object()["result"].toArray();
            qint64          satoshis=0;
            foreach(QJsonValue entry,result)
            {
                const QString   addr        =entry.toObject()["account"].toString();
                const QString   balanceStr  =entry.toObject()["balance"].toString();
                const qint64    balance     =balanceStr.left(balanceStr.length()-10).toLongLong();
                _addresses[addr]=entry.toObject();
                satoshis+=balance;
            }
            double          amount=static_cast<double>(satoshis)/100000000;
            setAsset("ETH","ethereum",amount);

        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed account (%1)").arg(statusCode));
    }
    delete(reply);
}


TM_CORE_END()
