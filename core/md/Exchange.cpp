#include "Exchange.h"
#include "ExchangeFeed.h"
#include "MarketData.h"
#include "Asset.h"

//
#define TAG         "md::Exchange"

TM_CORE_BEGIN(md)

//
Exchange::Exchange(const QString& id,const QString& name,bool feed,MarketData* marketData) : QObject(marketData)
{
    _id         =id;
    _name       =name;
    _marketData =marketData;
    if(feed)
    {
        _feed=new ExchangeFeed(marketData->connection(),_id,marketData);
    }
}

//
const QString&      Exchange::id()
{
    return(_id);
}

//
const QString&      Exchange::name()
{
    return(_name);
}

//
bool                Exchange::ready()
{
    if(_feed)
    {
        if(!_feed->ready())
            return(false);
    }
    return(true);
}

//
ExchangeFeed*       Exchange::feed()
{
    return(_feed);
}

//
QStringList         Exchange::quoteAssets()
{
    QStringList     list;
    if(_feed)
    {
        foreach(const ExchangeFeed::Ticker& ticker, _feed->tickers())
        {
            const QString   left        =ticker.name.split("-")[0];
            const QString   right       =ticker.name.split("-")[1];
            const QString   quoteAsset  =_marketData->loopupExchangeAsset(_id,right);
            /*if(!list.contains(left))
                list.append(left);*/
            if( (quoteAsset.length())&&(!list.contains(quoteAsset)) )
            {
                list.append(quoteAsset);
            }
        }
    }
    list.removeDuplicates();
    return(list);
}

//
QStringList         Exchange::quoteSymbols()
{
    QStringList     list;
    if(_feed)
    {
        foreach(const ExchangeFeed::Ticker& ticker, _feed->tickers())
        {
            const QString   left        =ticker.name.split("-")[0];
            const QString   right       =ticker.name.split("-")[1];
            list.append(right);
        }
    }
    list.removeDuplicates();
    return(list);
}

//
QStringList         Exchange::symbols()
{
    QStringList     list;
    if(_feed)
    {
        foreach(const ExchangeFeed::Ticker& ticker, _feed->tickers())
        {
            const QString   left    =ticker.name.split("-")[0];
            const QString   right   =ticker.name.split("-")[1];
            if(!list.contains(left))
                list.append(left);
            if(!list.contains(right))
                list.append(right);
        }
    }
    list.removeDuplicates();
    return(list);
}

//
QStringList         Exchange::unregisteredSymbols()
{
    QStringList     list;
    if(_feed)
    {
        list=symbols();
        foreach(auto assetID,_marketData->assets())
        {
            Asset*          asset   =_marketData->asset(assetID);
            const QString   symbol  =asset->exchange(_id);
            list.removeAll(symbol);
        }
    }
    return(list);
}

TM_CORE_END()
