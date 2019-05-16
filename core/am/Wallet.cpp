#include "Wallet.h"
#include "AssetsManager.h"

TM_CORE_BEGIN(am)

//
Wallet::Wallet(AssetsManager* assetsManager,QJsonObject config) : QObject(assetsManager)
{
    _assetsManager      =assetsManager;
    _id                 =config["id"].toInt();
    _name               =config["name"].toString();
    _config             =config;
}

//
QJsonObject                 Wallet::config()
{
    return(_config);
}

//
int                         Wallet::id()
{
    return(_id);
}

//
QString                     Wallet::name()
{
    return(_name);
}

//
AssetsManager*              Wallet::manager()
{
    return(_assetsManager);
}

//
tmio::md::MarketData*       Wallet::marketData()
{
    return(manager()->marketData());
}

//
QNetworkAccessManager*      Wallet::networkManager()
{
    return(manager()->networkManager());
}

//
void                        Wallet::resetAssets()
{
    _assets.clear();
    emit assetsChanged();
}

//
void                        Wallet::setAsset(QString symbol,QString assetID,double amount)
{
    Asset   asset;
    asset.symbol        =symbol;
    asset.assetID       =assetID;
    asset.totalAmount   =amount;
    _assets[symbol]=asset;
    emit assetsChanged();
}

//
QVector<Wallet::Asset>      Wallet::assets()
{
    return(QVector<Asset>::fromList(_assets.values()));
}

TM_CORE_END()
