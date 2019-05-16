#include "MarketData.h"
#include "ExchangeFeed.h"
#include "Exchange.h"
#include "Asset.h"
#include "Source.h"

TM_CORE_BEGIN(md)
#define TAG                     "md::MarketData"
#define API_URL_EXCHANGES       "marketdata/exchanges"
#define API_URL_ASSETS          "marketdata/assets"
#define API_URL_SAVE_ASSET      "marketdata/assets/%1"
#define API_URL_DELETE_ASSET    "marketdata/assets/%1"
#define WS_URL                  "marketdata/assets"

//
MarketData::MarketData(rest::Connection* connection,QObject *parent) : QObject(parent)
{
    _connection             =connection;
    // coinmap request (temporary, must move to asset)
    _coinmapRequest     =_connection->createRequest("marketdata/coinmap");
    connect(_coinmapRequest,&rest::Request::finished,this,&MarketData::onCoinmapResult);
    _coinmapRequest->get(false);
    // assets request
    _assetsRequest      =_connection->createRequest(API_URL_ASSETS);
    connect(_assetsRequest,&rest::Request::finished,this,&MarketData::onAssetsResult);
    _assetsRequest->get(false);
    // websocket for real time updates
    _wsUrl=QUrl(QString("%1/%2?token=%3").arg(connection->wsRoot()).arg(WS_URL).arg(connection->token()));
    connect(&_webSocket, &QWebSocket::connected, this, &MarketData::onWsConnected);
    connect(&_webSocket, &QWebSocket::disconnected, this, &MarketData::onWsDisconnected);
    connect(&_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&_webSocket, &QWebSocket::textMessageReceived,
            this, &MarketData::onWsTextMessageReceived);
    connect(&_webSocket, &QWebSocket::binaryMessageReceived,
            this, &MarketData::onWsBinaryMessageReceived);
    connectFeed();
    // timer
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    _timer->start(1000*10);
}

//
rest::Connection*               MarketData::connection()
{
    return(_connection);
}

//
bool                            MarketData::ready()
{
    if(_coinmapRequest)
        return(false);
    if(_assetsRequest)
        return(false);
    if(_exchangesRequest)
        return(false);
    foreach(Exchange* exchange, _exchanges.values())
    {
        if(!exchange->ready())
            return(false);
    }
    foreach(Source* source,_sources)
    {
        if(!source->ready())
            return(false);
    }
    foreach(Asset* asset,_assets)
    {
        if(!asset->ready())
            return(false);
    }
    return(true);
}

//
ExchangeFeed*                   MarketData::exchangeFeed(const QString& name)
{
    return(_exchanges.contains(name)?_exchanges[name]->feed():nullptr);
}

//
int                             MarketData::coinID(const QString& exchange,const QString& symbol)
{
    QString key=exchange+"."+symbol;
    if(_coinMap.contains(key))
    {
        return(_coinMap[key]);
    }
    return(0);
}

QList<QString>                  MarketData::exchanges()
{
    return(_exchanges.keys());
}

//
QString                         MarketData::loopupExchangeAsset(QString exchange,QString symbol)
{
    const QString key           =exchange+"."+symbol;
    if(_assetsLookup.contains(key))
    {
        return(_assetsLookup[key]);
    }
    //Q_ASSERT(symbol!="BTC");
    return("");
}

//
bool                            MarketData::favoriteTicker(const QString& exchange,const QString& ticker)
{
    const QString key="favorites/tickers/"+exchange+"/"+ticker;
    return(connection()->account()->accountProperty(key).toBool());
}

//
bool                            MarketData::setFavoriteTicker(const QString& exchange,const QString& ticker,bool favorite)
{
    if(favorite!=favoriteTicker(exchange,ticker))
    {
        const QString key="favorites/tickers/"+exchange+"/"+ticker;
        connection()->account()->setAccountProperty(key,favorite);
        TM_TRACE(TAG,QString("Changed ticker %1.%2 favorite: %3").arg(exchange).arg(ticker).arg(favorite));
        emit favoriteTickerChanged(exchange,ticker,favorite);
        return(true);
    }
    return(false);
}

//
bool                            MarketData::favoriteAsset(const QString& assetID)
{
    const QString key="favorites/assets/"+assetID;
    return(connection()->account()->accountProperty(key).toBool());
}

//
bool                            MarketData::setFavoriteAsset(const QString& assetID,bool favorite)
{
    if(favorite!=favoriteAsset(assetID))
    {
        const QString key="favorites/assets/"+assetID;
        connection()->account()->setAccountProperty(key,favorite);
        TM_TRACE(TAG,QString("Changed asset %1 favorite: %2").arg(assetID).arg(favorite));
        emit favoriteAssetChanged(assetID,favorite);
        return(true);
    }
    return(false);
}

//
QStringList                     MarketData::watchAssets()
{
    QStringList     watches;
    foreach(QString asset,assets())
    {
        if(watchAsset(asset))
            watches.append(asset);
    }
    return(watches);
}

//
bool                            MarketData::watchAsset(const QString& assetID)
{
    const QString key="watches/assets/"+assetID;
    return(connection()->account()->accountProperty(key).toBool());
}

//
bool                            MarketData::setWatchAsset(const QString& assetID,bool favorite)
{
    if(favorite!=watchAsset(assetID))
    {
        const QString key="watches/assets/"+assetID;
        connection()->account()->setAccountProperty(key,favorite);
        TM_TRACE(TAG,QString("Changed asset %1 watch: %2").arg(assetID).arg(favorite));
        emit watchAssetChanged(assetID,favorite);
        return(true);
    }
    return(false);
}

//
Exchange*                       MarketData::exchange(QString id)
{
    return(_exchanges.contains(id)?_exchanges[id]:nullptr);
}

//
QStringList                     MarketData::assets()
{
    return(_assets.keys());
}

//
QStringList                     MarketData::fiatAssets()
{
    QStringList list;
    foreach(Asset* asset,_assets.values())
    {
        if(asset->isFiat())
            list.append(asset->id());
    }
    return(list);
}

//
Asset*                          MarketData::asset(QString id)
{
    return(_assets.contains(id)?_assets[id]:nullptr);
}

//
bool                            MarketData::saveAsset(QString id)
{
    Q_ASSERT(_assets.contains(id));
    QJsonObject     jso     =asset(id)->save();
    rest::RequestResult     result=_connection->rPOST(QString(API_URL_SAVE_ASSET).arg(id),jso);
    if(result._statusCode==200)
    {
        return(true);
    }
    return(false);
}

//
Asset*                          MarketData::createAsset(QString id)
{
    if( (_assets.contains(id))||(id.isEmpty()) )
        return(nullptr);
    Asset*                  asset=new Asset(id,this);
    _assets.insert(id,asset);
    return(asset);
}

//
bool                            MarketData::deleteAsset(QString id)
{
    Q_ASSERT(_assets.contains(id));
    rest::RequestResult     result=_connection->rDELETERESOURCE(QString(API_URL_DELETE_ASSET).arg(id));
    if(result._statusCode==200)
    {
        emit assetDeleted(id);
        _assets[id]->deleteLater();
        _assets.remove(id);
        return(true);
    }
    return(false);
}

//
void                            MarketData::loadSources()
{
    if(_sources.size()==0)
    {
        if(_connection->account()->isMemberOf("marketdata"))
        {
            _sources.append(new Source(_connection,md::sourceCoinmarketcap));
            _sources.append(new Source(_connection,md::sourceCoingecko));
        }
        else
        {
            TM_ERROR(TAG,"Does not have provileges to load sources.");
        }
    }
    else
    {
        TM_ERROR(TAG,"Sources already loaded.");
    }
}

//
bool                            MarketData::sourcesReady()
{
    foreach(Source* source,_sources)
    {
        if(!source->ready())
            return(false);
    }
    return(true);
}

//
Source*                         MarketData::source(const QString& name)
{
    foreach(Source* s,_sources)
    {
        if(s->name()==name)
            return(s);
    }
    return(nullptr);
}

//
QStringList                     MarketData::availableSources()
{
    QStringList     list;
    foreach(Source* s,_sources)
    {
        list.append(s->name());
    }
    return(list);
}

//
void                            MarketData::connectFeed()
{
    if(!_wsConnecting)
    {
        TM_TRACE(TAG,QString("Connecting to %1").arg(_wsUrl.toString()));
        _webSocket.open(QUrl(_wsUrl));
        _wsConnecting=true;
    }
}

//
bool                            MarketData::updateAsset(QJsonObject jsAsset)
{
    const QString   id          =jsAsset["_id"].toString();
    Q_ASSERT(!id.isEmpty());
    Asset*          asset       =_assets.contains(id)?_assets[id]:nullptr;
    if(!asset)
    {
        asset=new Asset(id,this);
        _assets.insert(id,asset);
    }
    else
    {
        // remove old map from lookup
        foreach(QString exchange,asset->exchanges().keys())
        {
            const QString   key=QString("%1.%2").arg(exchange).arg(asset->exchange(exchange));
            _assetsLookup.remove(key);
        }
    }
    asset->load(jsAsset);
    // update lookup
    foreach(QString exchange,asset->exchanges().keys())
    {
        const QString   key=QString("%1.%2").arg(exchange).arg(asset->exchange(exchange));
        Q_ASSERT(!_assetsLookup.contains(key));
        _assetsLookup.insert(key,asset->id());
    }
    emit assetModified(id);
    return(true);
}

//
void                            MarketData::onCoinmapResult(tmio::rest::RequestResult& result)
{
    TM_TRACE(TAG,"Receive coin map");
    QJsonObject object=result._document.object();
    QStringList keys=object.keys();
    foreach(QString key, keys)
    {
        TM_TRACE(TAG,key+" map found");
        QJsonObject     map=object[key].toObject();
        QStringList     symbols=map.keys();
        foreach(const QString &symbol, symbols)
        {
            _coinMap[key+"."+symbol]=map[symbol].toInt();
        }
    }
    delete(_coinmapRequest);
    _coinmapRequest=nullptr;
}

//
void                            MarketData::onExchangesResult(tmio::rest::RequestResult& result)
{
    TM_TRACE(TAG,"Receive exchanges");
    QJsonArray  jsExchanges=result._document.array();
    for(int iExchange=0;iExchange<jsExchanges.size();iExchange++)
    {
        QJsonObject     jsExchange  =jsExchanges[iExchange].toObject();
        const QString   id          =jsExchange["_id"].toString();
        const QString   name        =jsExchange["name"].toString();
        const bool      feed        =jsExchange["feed"].toBool();
        Exchange*       exchange    =_exchanges.contains(id)?_exchanges[id]:nullptr;
        if(!exchange)
        {
            exchange    =new Exchange(id,name,feed,this);
            _exchanges.insert(id,exchange);
        }
    }
    delete(_exchangesRequest);
    _exchangesRequest=nullptr;
}

//
void                            MarketData::onAssetsResult(tmio::rest::RequestResult& result)
{
    TM_TRACE(TAG,"Receive assets");
    QJsonArray  jsAssets=result._document.array();
    for(int iAsset=0;iAsset<jsAssets.size();iAsset++)
    {
        QJsonObject     jsAsset     =jsAssets[iAsset].toObject();
        updateAsset(jsAsset);
    }
    delete(_assetsRequest);
    _assetsRequest=nullptr;
    // exchanges request
    _exchangesRequest   =_connection->createRequest(API_URL_EXCHANGES);
    connect(_exchangesRequest,&rest::Request::finished,this,&MarketData::onExchangesResult);
    _exchangesRequest->get(false);
}

//
void                            MarketData::onTimer()
{
    if(!_wsConnected)
    {
        connectFeed();
    }
}

//
void                            MarketData::onWsConnected()
{
    TM_TRACE(TAG,"Socket connected");
    _wsConnected    =true;
    _wsConnecting   =false;
}

//
void                            MarketData::onWsDisconnected()
{
    TM_TRACE(TAG,"Socket disconnected");
    _wsConnected    =false;
    _wsConnecting   =false;
}

//
void                            MarketData::onWsError(QAbstractSocket::SocketError /*error*/)
{
    TM_TRACE(TAG,"Socket error");
    _wsConnecting   =false;
}

//
void                            MarketData::onWsTextMessageReceived(QString message)
{
    // should not happen
    TM_TRACE(TAG,"TEXT MESSAGE RECEIVED");
    TM_TRACE(TAG,message);
}

//
void                            MarketData::onWsBinaryMessageReceived(const QByteArray& message)
{
    //TM_TRACE(TAG,"BINARY MESSAGE RECEIVED");
    QJsonObject             event=QJsonDocument::fromJson(message).object();
    if(event["type"].toString()=="updateAsset")
    {
        QJsonObject             data=event["data"].toObject();
        TM_TRACE(TAG,QString("Update asset %1").arg(data["_id"].toString()));
        updateAsset(data);
    }
    if(event["type"].toString()=="deleteAsset")
    {
        QJsonObject             data    =event["data"].toObject();
        const QString           assetID =data["_id"].toString();
        TM_TRACE(TAG,"Delete asset");
        TM_TRACE(TAG,QJsonDocument(data).toJson());
        if(_assets.contains(assetID))
        {
            _assets[assetID]->deleteLater();
            _assets.remove(assetID);
            emit assetDeleted(assetID);
        }
    }
}



TM_CORE_END()
