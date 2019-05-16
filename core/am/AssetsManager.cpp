#include <QtNetwork/QNetworkConfiguration>
#include "Wallet.h"
#include "AssetsManager.h"
#include "MiningWallet_Nicehash.h"
#include "MiningWallet_Ethermine.h"
#include "ManagedWallet_Binance.h"
#include "ManagedWallet_Bitstamp.h"
#include "ManagedWallet_Kraken.h"
#include "ManagedWallet_Coinbase.h"
#include "PrivateWallet_Blockchain.h"
#include "PrivateWallet_Etherscan.h"

#include "../md/Asset.h"

//
#define TAG "tmio::am::AssetsManager"

//
TM_CORE_BEGIN(am)

//
AssetsManager::AssetsManager(tmio::md::MarketData* marketData) : QObject(nullptr)
{
    _marketData=marketData;
    _networkManager=new QNetworkAccessManager(this);
}

//
void                                AssetsManager::reset()
{
    for(int i=0;i<_wallets.size();i++)
    {
        delete(_wallets[i]);
    }
    _wallets.clear();
    emit walletsChanged();
}

//
tmio::md::MarketData*               AssetsManager::marketData()
{
    return(_marketData);
}

//
QVector<Wallet*>                    AssetsManager::wallets()
{
    return(_wallets);
}

//
void                                AssetsManager::addWallet(QJsonObject config)
{
    Wallet*     wallet=nullptr;
    /* Exchange         */
    if(config["exchange"].isObject())
    {
        QJsonObject                 exchangeWallet=config["exchange"].toObject();
        if(exchangeWallet["binance"].isObject())
        {
            wallet=new ManagedWallet_Binance(this,config);
        }
        if(exchangeWallet["coinbase"].isObject())
        {
            wallet=new ManagedWallet_Coinbase(this,config);
        }
        if(exchangeWallet["kraken"].isObject())
        {
            wallet=new ManagedWallet_Kraken(this,config);
        }
        if(exchangeWallet["bitstamp"].isObject())
        {
            wallet=new ManagedWallet_Bitstamp(this,config);
        }
    }
    /* Mining           */
    if(config["mining"].isObject())
    {
        QJsonObject                 miningWallet=config["mining"].toObject();
        if(miningWallet["nicehash"].isObject())
        {
            wallet=new MiningWallet_Nicehash(this,config);
        }
        if(miningWallet["ethermine"].isObject())
        {
            wallet=new MiningWallet_Ethermine(this,config);
        }
    }
    /* Private          */
    if(config["private"].isObject())
    {
        QJsonObject                 privateWallet=config["private"].toObject();
        if(privateWallet["bitcoin"].isObject())
        {
            wallet=new PrivateWallet_Blockchain(this,config);
        }
        if(privateWallet["ethereum"].isObject())
        {
            wallet=new PrivateWallet_Etherscan(this,config);
        }
    }
    if(wallet)
    {
        connect(wallet,&Wallet::assetsChanged,this,&AssetsManager::onWalletAssetsChanged);
        _wallets.append(wallet);
        emit walletsChanged();
    }
    else
    {
        TM_ERROR(TAG,"Unable to add wallet: "+config["name"].toString());
    }
}

//
QList<Wallet::Asset>                AssetsManager::assets()
{
    QMap<QString,Wallet::Asset>     allAssets;
    foreach(am::Wallet* wallet, wallets())
    {
        QVector<tmio::am::Wallet::Asset>      assets            =wallet->assets();
        foreach(am::Wallet::Asset asset, assets)
        {
            const QString       assetID=asset.assetID.length()?asset.assetID:asset.symbol;
            if(!allAssets.contains(assetID))
            {
                if(asset.assetID.length())
                {
                    asset.symbol    =_marketData->asset(asset.assetID)->symbol();
                }
                allAssets.insert(assetID,asset);
            }
            else
            {
                allAssets[assetID].totalAmount+=asset.totalAmount;
            }
        }
    }
    return(allAssets.values());
}

//
QNetworkAccessManager*              AssetsManager::networkManager()
{
    return(_networkManager);
}

//
QNetworkRequest*                    AssetsManager::createNetworkRequest(const QUrl& url)
{
    QNetworkRequest*        req=new QNetworkRequest(url);
    return(req);
}

//
void                                AssetsManager::onWalletAssetsChanged()
{
    emit walletsChanged();
}

TM_CORE_END()
