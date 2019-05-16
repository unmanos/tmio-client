#include <rest/Request.h>
#include "Asset.h"
#include "MarketData.h"

TM_CORE_BEGIN(md)

#define API_ASSET_ICON_URL          "marketdata/assets/%1/icon"

const QString                       Asset::BTC          ="bitcoin";
const QString                       Asset::LTC          ="litecoin";
const QString                       Asset::ETH          ="ethereum";
const QString                       Asset::XMR          ="monero";
const QString                       Asset::EUR          ="fiat-eur";
const QString                       Asset::USD          ="fiat-usd";
const QString                       Asset::typeFiat     ="fiat";
const QString                       Asset::typeCurrency ="currency";
const QString                       Asset::typeToken    ="token";

//
Asset::Asset(const QString& id,MarketData *marketData) : QObject(marketData)
{
    _marketData =marketData;
    _id         =id;
}

//
const QString&                      Asset::id()
{
    return(_id);
}

//
bool                                Asset::ready()
{
    return(_iconRequest==nullptr);
}

//
const QString&                      Asset::name()
{
    return(_name);
}

//
void                                Asset::setName(const QString& name)
{
    _name=name;
}

//
const QString&                      Asset::sign()
{
    return(_sign);
}

//
const QString&                      Asset::symbol()
{
    return(_symbol);
}

//
void                                Asset::setSymbol(const QString& symbol)
{
    _symbol=symbol;
}

//
const QString&                      Asset::type()
{
    return(_type);
}

//
bool                                Asset::isFiat()
{
    return(_type==typeFiat);
}

//
void                                Asset::setType(const QString& type)
{
    _type=type;
}

//
const QMap<QString,QString>&        Asset::sources()
{
    return(_sources);
}

//
const QString&                      Asset::source(const QString& name)
{
    if(_sources.contains(name))
    {
        return(_sources[name]);
    }
    static QString empty="";
    return(empty);
}

//
void                                Asset::setSource(const QString& name,const QString& coinId)
{
    _sources[name]=coinId;
}

//
const QMap<QString,QString>&        Asset::exchanges()
{
    return(_exchanges);
}

//
const QString&                      Asset::exchange(const QString& name)
{
    if(_exchanges.contains(name))
    {
        return(_exchanges[name]);
    }
    static QString empty="";
    return(empty);
}

//
void                                Asset::setExchange(const QString& name,const QString& symbol)
{
    _exchanges[name]=symbol;
}

//
QIcon                               Asset::icon()
{
    if( (!_iconLoaded)&&(!_iconRequest) )
    {
        _iconRequest        =_marketData->connection()->createRequest(QString(API_ASSET_ICON_URL).arg(_id));
        connect(_iconRequest,&rest::Request::finished,this,&Asset::onReceiveIcon);
        _iconRequest->get(false);
        _iconLoaded         =true;
        //_icon               =QIcon();
    }
    return(_icon);
}

//
void                                Asset::load(QJsonObject asset)
{
    Q_ASSERT(_id==asset["_id"].toString());
    _name       =asset["name"].toString();
    _type       =asset["type"].toString();
    _symbol     =asset["symbol"].toString();
    _sign       =asset["sign"].toString();
    // read sources
    QJsonObject sources=asset["sources"].toObject();
    _sources.clear();
    foreach(QString source,sources.keys())
    {
        _sources.insert(source,sources[source].toString());
    }
    // read exchanges
    QJsonObject exchanges=asset["exchanges"].toObject();
    _exchanges.clear();
    foreach(QString exchange,exchanges.keys())
    {
        _exchanges.insert(exchange,exchanges[exchange].toString());
    }
    // read source contents
    if(asset.contains("data"))
    {
        QJsonObject data=asset["data"].toObject();
        _data=data;
    }
    // invalidate icon
    _iconLoaded  =false;
}

//
QJsonObject                         Asset::save()
{
    QJsonObject object;
    object.insert("_id",_id);
    object.insert("name", _name);
    object.insert("type", _type);
    object.insert("symbol", _symbol);
    if(_sign.length())
    {
        object.insert("sign", _sign);
    }
    // save sources
    QJsonObject sources;
    foreach(QString source,_sources.keys())
    {
        if(!_sources[source].isEmpty())
        {
            sources.insert(source, _sources[source]);
        }
    }
    object.insert("sources", sources);
    // save exchanges
    QJsonObject exchanges;
    foreach(QString exchange,_exchanges.keys())
    {
        if(!_exchanges[exchange].isEmpty())
        {
            exchanges.insert(exchange, _exchanges[exchange]);
        }
    }
    object.insert("exchanges", exchanges);
    return(object);
}

//
void                                Asset::onReceiveIcon(rest::RequestResult& result)
{
    if(result._statusCode==200)
    {
        QImage image;
        image.loadFromData(result._raw);
        QPixmap buffer = QPixmap::fromImage(image);
        _icon.addPixmap(buffer);
    }
    // set icon
    delete(_iconRequest);
    _iconRequest=nullptr;
}

//
void                                Asset::updatePrice(const QString& exchange,const QDateTime& time,const QString& quoteID,double price,bool reverse)
{
    const QString       key=exchange+"."+quoteID;
    Quote               quote;
    quote.time          =time;
    quote.exchange      =exchange;
    quote.quoteID       =quoteID;
    quote.price         =price;
    quote.reverse       =reverse;
    _quotes[key]        =quote;
    emit priceChanged();
}

//
int                                 Asset::getPrice(const QString& quoteID,double& price)
{
    int     averageCount=0;
    price=0.0;
    foreach(const Quote& quote,_quotes)
    {
        if(quote.quoteID==quoteID)
        {
            price+=quote.price;
            averageCount++;
        }
    }
    if(averageCount)
    {
        price/=static_cast<double>(averageCount);
    }
    return(averageCount);
}

//
QDateTime                           Asset::lastPriceUpdate()
{
    QDateTime   dt;
    //bool        init=false;
    foreach(const Quote& quote,_quotes)
    {
        if( (quote.reverse)&&(quote.quoteID!=Asset::BTC) )
            continue;
        if(dt.isNull())
            dt=quote.time;
        else
            dt=qMax(dt,quote.time);
    }
    return(dt);
}

//
const QMap<QString,Asset::Quote>&   Asset::quotes()
{
    return(_quotes);
}

//
QJsonObject                         Asset::data()
{
    return(_data);
}

TM_CORE_END()
