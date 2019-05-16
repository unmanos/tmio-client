#include <QtWebSockets/QWebSocket>
#include <QDataStream>
#include "ExchangeFeed.h"
#include "MarketData.h"
#include "Asset.h"

#define TAG                     "md::ExchangeFeed"
#define WS_RECEIVE_TIMEOUT      (60*2)

TM_CORE_BEGIN(md)

//
ExchangeFeed::ExchangeFeed(rest::Connection* connection,const QString& name,MarketData* marketData) : QObject(marketData)
{
    // get all tickers
    _webSocket      =connection->createWebSocket();
    _marketData     =marketData;
    _name           =name;
    _wsReceivedTime =QDateTime::currentDateTime();

    _tickerRequest=connection->createRequest(QString("marketdata/exchanges/%1/tickers").arg(name));
    connect(_tickerRequest, SIGNAL(finished(tmio::rest::RequestResult&)), this, SLOT(onReceiveTickers(tmio::rest::RequestResult&)));
    _tickerRequest->get(false);
    // websocket for real time updates
    _wsUrl=QUrl(QString("%1/marketdata/exchanges/feeds/%2?token=%3").arg(connection->wsRoot()).arg(name).arg(connection->token()));
    connect(_webSocket, &QWebSocket::connected, this, &ExchangeFeed::onConnected);
    connect(_webSocket, &QWebSocket::disconnected, this, &ExchangeFeed::onDisconnected);
    connect(_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(_webSocket, &QWebSocket::textMessageReceived,this, &ExchangeFeed::onTextMessageReceived);
    connect(_webSocket, &QWebSocket::binaryMessageReceived,this, &ExchangeFeed::onBinaryMessageReceived);
    TM_TRACE(TAG,_name+QString(" - connecting socket"));
    connectFeed();
    // timer
    connect(&_autoReconnectTimer, SIGNAL(timeout()), this, SLOT(onAutoReconnect()));
    _autoReconnectTimer.start(1000*20);
}

//
bool                            ExchangeFeed::ready()
{
    if(_tickerRequest)
        return(false);
    if(!_ready)
        return(false);
    return(true);
}

//
const QString&                  ExchangeFeed::name()
{
    return(_name);
}

//
QVector<ExchangeFeed::Ticker>   ExchangeFeed::tickers()
{
    QVector<ExchangeFeed::Ticker>   tickers;
    QMapIterator<QString, ExchangeFeed::Ticker> i(_tickers);
    while (i.hasNext())
    {
        i.next();
        tickers.append(i.value());
    }
    return(tickers);
}

//
ExchangeFeed::Ticker            ExchangeFeed::ticker(const QString& name)
{
    Q_ASSERT(_tickers.contains(name));
    return(_tickers[name]);
}

//
void                            ExchangeFeed::connectFeed()
{
    if(!_connecting)
    {
        _webSocket->open(QUrl(_wsUrl));
        _connecting=true;
        TM_TRACE(TAG,QString("Connecting to %1").arg(_wsUrl.toString()));
    }
}

//
void                            ExchangeFeed::updateTicker(const QJsonObject& json)
{
    Ticker      t;
    t.json=json;
    t.name=json["_id"].toString();
    t.price=json["price"].toDouble();
    t.time=QDateTime::fromString(json.value("time").toString(),Qt::ISODate);
    // lookup asset
    QStringList         pair    =t.name.split("-");
    if(pair.length()==2)
    {
        const QString       assetID =_marketData->loopupExchangeAsset(_name,pair.at(0));
        const QString       quoteID =_marketData->loopupExchangeAsset(_name,pair.at(1));
        if( (assetID.length()==0)||(quoteID.length()==0) )
        {
            TM_WARNING(TAG,QString("%1.%2 map to %3 / %4").arg(_name).arg(t.name).arg(assetID).arg(quoteID));
        }
        t.assetID       =assetID;
        t.quoteID       =quoteID;
        // update price
        Asset*          asset       =_marketData->asset(assetID);
        Asset*          quoteAsset  =_marketData->asset(quoteID);
        if( (asset)&&(quoteAsset) )
        {
            asset->updatePrice(_name,t.time,quoteID,t.price,false);
            quoteAsset->updatePrice(_name,t.time,assetID,1.0/t.price,true);
            //asset->updatePrice(_name,quoteID,t.price);
        }
    }
    // previous prices
    QJsonArray      previousPrices=json["previous"].toArray();
    for(int pprow=0;pprow<previousPrices.count();pprow++)
    {
        QJsonObject pp=previousPrices.at(pprow).toObject();
        t.previousPrices[pp["interval"].toInt()]=pp["price"].toDouble();
    }
    // period volumes
    QJsonArray      volumes=json["volumes"].toArray();
    for(int vrow=0;vrow<volumes.count();vrow++)
    {
        QJsonObject v=volumes.at(vrow).toObject();
        t.quoteVolumes[v["interval"].toInt()]=v["quoteVolume"].toDouble();
        t.volumes[v["interval"].toInt()]=v["volume"].toDouble();
    }
    // update
    if(_tickers.contains(t.name))
    {
        _tickers[t.name]=t;
        //qDebug() << "Upadte " << t.name;
    }
    else
    {
        _tickers.insert(t.name,t);
        //qDebug() << "Insert " << t.name;
    }
    emit tickerChanged(t);
    TM_TRACE(TAG,QString("Update %1 => %2").arg(t.name).arg(t.price));
}

//
void                            ExchangeFeed::onAutoReconnect()
{
    //TM_TRACE(TAG,QString("Auto reconnect timer check: %1").arg(elapsed));
    if(!_connected)
    {
        connectFeed();
    }
    else
    {
        if(_wsReceivedTime.secsTo(QDateTime::currentDateTime())>WS_RECEIVE_TIMEOUT)
        {
            TM_WARNING(TAG,"Connection timedout, disconnecting.");
            _webSocket->close();
        }
    }
}

//
void                            ExchangeFeed::onConnected()
{
    TM_TRACE(TAG,"Socket connected");
    _connected      =true;
    _ready          =false;
    _connecting     =false;
    _wsReceivedTime =QDateTime::currentDateTime();
    _shortIdLookup.clear();
}

//
void                            ExchangeFeed::onDisconnected()
{
    TM_TRACE(TAG,_name+" - socket disconnected");
    _connected          =false;
    _connecting         =false;
}

//
void                            ExchangeFeed::onError(QAbstractSocket::SocketError /*error*/)
{
    TM_TRACE(TAG,_name+" - socket error");
    _connecting     =false;
}

//
void                            ExchangeFeed::onTextMessageReceived(QString message)
{
    TM_TRACE(TAG,message);
    _wsReceivedTime =QDateTime::currentDateTime();
}

//
void                            ExchangeFeed::onBinaryMessageReceived(const QByteArray& message)
{
    QDataStream     stream(message);
    stream.setVersion(QDataStream::Qt_4_8);
    quint8          messageType;
    stream >> messageType;
    switch(messageType)
    {
        // full update
        case 0x01:
        case 0x02:
        {
            //TM_TRACE(TAG,"FULL");
            QString     id;
            quint16     shortId;
            qint32      time;
            double      price;
            if(messageType==0x01)
            {
                stream >> id;
                stream >> shortId;
            }
            else
            {
                stream >> shortId;
                id=_shortIdLookup[shortId];
                Q_ASSERT(_shortIdLookup.contains(shortId));
            }
            stream >> time;
            stream >> price;
            Ticker      t;
            t.name      =id;
            t.time      =QDateTime::fromSecsSinceEpoch(time);
            t.price     =price;
            QStringList         pair    =t.name.split("-");
            if(pair.length()==2)
            {
                const QString       assetID =_marketData->loopupExchangeAsset(_name,pair.at(0));
                const QString       quoteID =_marketData->loopupExchangeAsset(_name,pair.at(1));
                if( (assetID.length()==0)||(quoteID.length()==0) )
                {
                    TM_WARNING(TAG,QString("%1.%2 map to %3 / %4").arg(_name).arg(t.name).arg(assetID).arg(quoteID));
                }
                t.assetID       =assetID;
                t.quoteID       =quoteID;
                // update price
                Asset*          asset       =_marketData->asset(assetID);
                Asset*          quoteAsset  =_marketData->asset(quoteID);
                if( (asset)&&(quoteAsset) )
                {
                    asset->updatePrice(_name,t.time,quoteID,t.price,false);
                    quoteAsset->updatePrice(_name,t.time,assetID,1.0/t.price,true);
                }
            }
            // previous prices&volumes
            while(!stream.atEnd())
            {
                quint8          varId;
                stream >> varId;
                switch(varId)
                {
                    // previous price (short interval)
                    case 0x02:
                    {
                        quint16     shortInterval;
                        float       price;
                        stream >> shortInterval;
                        stream >> price;
                        qint32      interval=static_cast<qint32>(shortInterval)*6000;
                        t.previousPrices[interval]=static_cast<double>(price);
                    }
                    break;
                    // previous price (long interval)
                    case 0x03:
                    {
                        qint32      interval;
                        float       price;
                        stream >> interval;
                        stream >> price;
                        t.previousPrices[interval]=static_cast<double>(price);
                    }
                    break;
                    // volume (short interval)
                    case 0x12:
                    {
                        quint16     shortInterval;
                        float       quoteVolume;
                        stream >> shortInterval;
                        stream >> quoteVolume;
                        qint32      interval=static_cast<qint32>(shortInterval)*6000;
                        t.quoteVolumes[interval]=static_cast<double>(quoteVolume);
                    }
                    break;
                    // volume (long interval)
                    case 0x13:
                    {
                        qint32      interval;
                        float       volume;
                        stream >> interval;
                        stream >> volume;
                        t.quoteVolumes[interval]=static_cast<double>(volume);
                        TM_TRACE(TAG,QString("Long volme interval %1").arg(interval));
                    }
                    break;
                    //
                }
            }
            _tickers[t.name]            =t;
            _shortIdLookup[shortId]     =id;
            emit tickerChanged(t);
        }
        break;
        // short update
        case 0x03:
        {
            //TM_TRACE(TAG,"SHORT");
            quint16     shortId;
            qint32      time;
            double      price;
            stream >> shortId;
            stream >> time;
            stream >> price;
            Q_ASSERT(_shortIdLookup.contains(shortId));
            Ticker      t=_tickers[_shortIdLookup[shortId]];
            t.time      =QDateTime::fromSecsSinceEpoch(time);
            t.price     =price;
            // update price
            Asset*          asset       =_marketData->asset(t.assetID);
            Asset*          quoteAsset  =_marketData->asset(t.quoteID);
            if( (asset)&&(quoteAsset) )
            {
                asset->updatePrice(_name,t.time,t.quoteID,t.price,false);
                quoteAsset->updatePrice(_name,t.time,t.assetID,1.0/t.price,true);
            }
            // remove old volume <1m
            if(t.quoteVolumes.count())
            {
                if(t.quoteVolumes.firstKey()<6000)
                {
                    TM_TRACE(TAG,QString("Removed volume %1").arg(t.quoteVolumes.firstKey()));
                    t.quoteVolumes.remove(t.quoteVolumes.firstKey());
                }
            }

            _tickers[t.name]            =t;
            emit tickerChanged(t);
        }
        break;
        // ready
        case 0xFF:
        _ready=true;
        break;
        //
    }
    _wsReceivedTime =QDateTime::currentDateTime();
}

//
void                            ExchangeFeed::onReceiveTickers(rest::RequestResult& result)
{
    TM_TRACE(TAG,_name+" - received tickers");
    QJsonArray          jsonArray=result._document.array();
    for(int row=0;row<jsonArray.count();row++)
    {
        QJsonObject         jsTicker    =jsonArray.at(row).toObject();
        updateTicker(jsTicker);
    }
    delete(_tickerRequest);
    _tickerRequest=nullptr;
}

TM_CORE_END()
