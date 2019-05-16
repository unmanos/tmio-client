#include "CoinMarketCap.h"
#include "Application.h"

#include <Core.h>
#include <QTimer>
#include <rest/Connection.h>
#include <rest/Request.h>
#include <md/Asset.h>

//
#define TAG "md.CoinMarketCap"

//
CoinMarketCap::CoinMarketCap(QObject* parent) : QObject(parent)
{
    TM_TRACE(TAG,"Init CMC");
    _quoteAssetID=Application::instance().settings().value(SETTINGS_QUOTE_ASSET_ID,"bitcoin").toString();
}

//
bool                                CoinMarketCap::ready()
{
    return(true);
}

//
QString                             CoinMarketCap::quoteAssetID()
{
    return(_quoteAssetID);
}

//
QString                             CoinMarketCap::quoteAssetSymbol()
{
    md::Asset*      asset=Application::instance().marketData()->asset(_quoteAssetID);
    return(asset?asset->symbol():"");
}

//
void                                CoinMarketCap::setQuoteAssetID(const QString& fiatID)
{
    if(_quoteAssetID!=fiatID)
    {
        _quoteAssetID=fiatID;
        Application::instance().settings().setValue(SETTINGS_QUOTE_ASSET_ID,_quoteAssetID);
        emit quoteAssetChanged();
    }
}

//
QString                             CoinMarketCap::formatFiatCurrency(double amount,QString fiatID)
{
    int     precision=0;
    if(amount<1000000)
    {
        precision=2;
    }
    if(amount<5)
    {
        precision=4;
    }
    if(amount<0.5)
    {
        precision=6;
    }
    if(amount<0.05)
    {
        precision=8;
    }
    if( (amount<=1)&&(fiatID=="bitcoin") )
    {
        precision=8;
    }
    md::Asset*          asset=Application::instance().marketData()->asset(fiatID);
    const QString       symbol=asset->sign().length()?asset->sign():(asset->symbol()+" ");
    return(QLocale::system().toCurrencyString(amount,symbol,precision));
}

//
QString                             CoinMarketCap::formatAmount(double amount,double precision)
{
    if(amount<0)
        amount=0;
    return(QLocale::system().toString(amount,'f',precision));
}

//
QString                             CoinMarketCap::formatSupply(double supply)
{
    return(formatAmount(supply,0));
}

//
bool                                CoinMarketCap::assetPrice(const QString& quoteAssetID,const QString& assetID,double& amount,QVector<QString>* route,int recurse)
{
    //md::Asset*          asset=Application::instance().marketData()->asset(fiatID);
    md::MarketData*     marketData  =Application::instance().marketData();
    md::Asset*          asset       =marketData->asset(assetID);
    double              price;
    if(quoteAssetID==assetID)
    {
        return(true);
    }
    if(asset->getPrice(quoteAssetID,price))
    {
        amount=price*amount;
        return(true);
    }
    // try to convert a quote to target
    if(recurse)
    {
        recurse--;
        // find matching quotes
        QVector<md::Asset::Quote>   quotes;
        foreach(const md::Asset::Quote& quote,asset->quotes())
        {
            if( (!quote.reverse)||(quote.quoteID==md::Asset::BTC)||(quote.quoteID==md::Asset::USD) )
            {
                if(quote.quoteID==md::Asset::BTC)
                {
                    quotes.push_front(quote);
                }
                else
                {
                    quotes.append(quote);
                }
            }
        }
        foreach(const md::Asset::Quote& quote,quotes)
        {
            if( (!quote.reverse)||(quote.quoteID==md::Asset::BTC)||(quote.quoteID==md::Asset::USD) )
            {
                md::Asset*      quoteAsset  =marketData->asset(quote.quoteID);
                double          quoteValue  =quote.price*amount;
                if(route) route->append(quoteAsset->id());
                if(assetPrice(quoteAssetID,quoteAsset->id(),quoteValue,route,recurse))
                {
                    amount=quoteValue;
                    return(true);
                }
                if(route) route->removeLast();
            }
        }
    }
    return(false);
}

//
double                              CoinMarketCap::circulatingSupply(const QString& assetID)
{
    md::Asset*      asset=Application::instance().marketData()->asset(assetID);
    if(asset->data().contains("supply"))
    {
        QJsonObject     supply=asset->data()["supply"].toObject();
        if(supply.contains("circulating"))
        {
            auto circulatingSupply=supply["circulating"].toDouble();
            return(static_cast<double>(circulatingSupply));
        }
    }
    return(0.0);
}

//
double                              CoinMarketCap::totalMarketCap(const QString& quoteAssetID)
{
    QTime myTimer;
    myTimer.start();
    md::MarketData*                 marketData=Application::instance().marketData();
    double totalMarketCap=0;
    QMap<double,QString>            rankSort;
    _assetRanks.clear();
    foreach(QString assetID,marketData->assets())
    {
        md::Asset*      asset=marketData->asset(assetID);
        if(!asset->isFiat())
        {
            double supply    =circulatingSupply(assetID);
            double price     =1;
            if( (supply>0.0001)&&(assetPrice(quoteAssetID,asset->id(),price,nullptr)) )
            {
                double      cap=supply*price;
                totalMarketCap+=cap;
                rankSort.insert(cap,assetID);
            }
        }
    }
    int rank=rankSort.count();
    foreach(QString assetID,rankSort.values())
    {
        _assetRanks.insert(assetID,rank--);
    }
    int nMilliseconds = myTimer.elapsed();
    if(nMilliseconds>5)
    {
        TM_WARNING(TAG,QString("totalMarketCap nMilliseconds: %1").arg(nMilliseconds));
    }
    return(totalMarketCap);
}

//
QIcon                               CoinMarketCap::assetIcon(const QString assetID)
{
    md::Asset*      asset=Application::instance().marketData()->asset(assetID);
    if(asset)
    {
        if(asset->type()=="fiat")
        {
            return(QIcon(":/icons/fiat/"+asset->symbol().toLower()+".png"));
        }
        return(asset->icon());
    }
    return(QIcon());
}

//
int                                 CoinMarketCap::assetRank(const QString assetID)
{
    if(_assetRanks.contains(assetID))
        return(_assetRanks[assetID]);
    return(0);
}

//
QIcon                               CoinMarketCap::exchangeIcon(const QString exchangeID)
{
    return(QIcon(":/icons/exchanges/"+exchangeID+".png"));
}

//
QIcon                               CoinMarketCap::miningPoolIcon(const QString miningPoolID)
{
    return(QIcon(":/icons/mining/"+miningPoolID+".png"));
}

//
QString                             CoinMarketCap::miningPoolName(const QString miningPoolID)
{
    if(miningPoolID=="nicehash")
        return("Nicehash");
    if(miningPoolID=="ethermine")
        return("Ethermine");
    Q_ASSERT(0);
    return("");
}

