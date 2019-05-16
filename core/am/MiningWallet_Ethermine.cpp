#include <QTimer>
#include <QtNetwork/QNetworkReply>
#include "MiningWallet_Ethermine.h"
#include <md/Asset.h>

//
#define TAG         "tmio::am::MiningWallet_Ethermine"
#define API_STATS   "https://api.ethermine.org/miner/%1/currentStats"

//
TM_CORE_BEGIN(am)

//
MiningWallet_Ethermine::MiningWallet_Ethermine(AssetsManager* assetsManager,QJsonObject config) : MiningWallet(assetsManager,config)
{
    _address        =config["mining"].toObject()["ethermine"].toObject()["address"].toString();
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&MiningWallet_Ethermine::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
void                    MiningWallet_Ethermine::updateBalance()
{
    TM_TRACE(TAG,"Update balance");
    if(!_balanceRequest)
    {
        //
        QString url=QString(API_STATS).arg(_address);
        TM_TRACE(TAG,url);
        _balanceRequest=new QNetworkRequest(QUrl(url));
        QNetworkReply*      reply=networkManager()->get(*_balanceRequest);
        connect(reply, &QNetworkReply::finished, this, &MiningWallet_Ethermine::onReceiveBalance);
    }
}

//
void                    MiningWallet_Ethermine::onUpdate()
{
    updateBalance();
}

//
void                    MiningWallet_Ethermine::onReceiveBalance()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        TM_TRACE(TAG,"Received balance");
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            if(doc.object().contains("data"))
            {
                resetAssets();
                QJsonObject data    =doc.object()["data"].toObject();
                qint64      unpaid  =data["unpaid"].toVariant().toLongLong();
                setAsset("ETH","ethereum",static_cast<double>(unpaid)/1000000000000000000.0);
            }
            else
            {
                TM_ERROR(TAG,"Response does not contain any result");
                TM_ERROR(TAG,raw.toStdString().c_str());
            }
        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed balance (%1)").arg(statusCode));
    }
    delete(_balanceRequest);
    _balanceRequest=nullptr;
    delete(reply);
}


TM_CORE_END()
