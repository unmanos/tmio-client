#include <QTimer>
#include <QtNetwork/QNetworkReply>
#include "MiningWallet_Nicehash.h"

#include <md/Asset.h>

//
#define TAG "tmio::am::ManagedWallet_Nicehash"

//
TM_CORE_BEGIN(am)

//
MiningWallet_Nicehash::MiningWallet_Nicehash(AssetsManager* assetsManager,QJsonObject config) : MiningWallet(assetsManager,config)
{
    _apiID      =config["mining"].toObject()["nicehash"].toObject()["apiID"].toString();
    _apiKey     =config["mining"].toObject()["nicehash"].toObject()["apiKey"].toString();
    _balanceRequest=NULL;
    _confirmedBalance=-1;
    _pendingBalance=-1;
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&MiningWallet_Nicehash::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
void                    MiningWallet_Nicehash::updateBalance()
{
    TM_TRACE(TAG,"Update balance");
    if(!_balanceRequest)
    {
        //
        QString url=QString("https://api.nicehash.com/api?method=balance&id=%1&key=%2").arg(_apiID).arg(_apiKey);
        TM_TRACE(TAG,url);
        _balanceRequest=new QNetworkRequest(QUrl(url));
        QNetworkReply*      reply=networkManager()->get(*_balanceRequest);
        connect(reply, &QNetworkReply::finished, this, &MiningWallet_Nicehash::onReceiveBalance);
    }
}

//
void                    MiningWallet_Nicehash::onUpdate()
{
    updateBalance();
}

//
void                    MiningWallet_Nicehash::onReceiveBalance()
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
            if(doc.object().contains("result"))
            {
                resetAssets();
                QJsonObject result=doc.object()["result"].toObject();
                _confirmedBalance=result["balance_confirmed"].toString().toDouble();
                _pendingBalance=result["balance_pending"].toString().toDouble();
                TM_TRACE(TAG,QString("Confirmed balance: %1").arg(_confirmedBalance));
                TM_TRACE(TAG,QString("Pending balance: %1").arg(_pendingBalance));
                setAsset("BTC",md::Asset::BTC,_confirmedBalance);
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
