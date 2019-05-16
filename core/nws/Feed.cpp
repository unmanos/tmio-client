#include "Feed.h"
TM_CORE_BEGIN(nws)

#define TAG                     "nws::Feed"
#define WS_URL                  "news"

#define MAX_MESSAGES_PER_SECOND 1

//
Feed::Feed(rest::Connection* connection,QObject *parent) : QObject(parent)
{
    _connection             =connection;
    // websocket for real time updates
    _wsUrl=QUrl(QString("%1/%2?token=%3").arg(connection->wsRoot()).arg(WS_URL).arg(connection->token()));
    connect(&_webSocket, &QWebSocket::connected, this, &Feed::onWsConnected);
    connect(&_webSocket, &QWebSocket::disconnected, this, &Feed::onWsDisconnected);
    connect(&_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&_webSocket, &QWebSocket::textMessageReceived,
            this, &Feed::onWsTextMessageReceived);
    connect(&_webSocket, &QWebSocket::binaryMessageReceived,
            this, &Feed::onWsBinaryMessageReceived);
    connectFeed();
    // timers
    connect(&_autoReconnectTimer, SIGNAL(timeout()), this, SLOT(onAutoReconnect()));
    _autoReconnectTimer.start(30*1000);
    connect(&_processMessagesTimer, SIGNAL(timeout()), this, SLOT(onProcessMessages()));
    _processMessagesTimer.start(1000);
}

//
QList<Feed::Item>               Feed::items()
{
    return(_items.values());
}

//
void                            Feed::connectFeed()
{
    if(!_wsConnecting)
    {
        TM_TRACE(TAG,QString("Connecting to %1").arg(_wsUrl.toString()));
        _webSocket.open(QUrl(_wsUrl));
        _wsConnecting=true;
    }
}

//
void                            Feed::updateNews(QJsonObject item)
{
    const QString       id              =item["_id"].toString();
    if(!_items.contains(id))
    {
        Item                i;
        i.feedTitle         =item["feedTitle"].toString();
        i.title             =item["title"].toString();
        i.description       =item["description"].toString();
        i.link              =item["link"].toString();
        i.pubDate           =item["pubDate"].toVariant().toDateTime();
        QByteArray          mediaContents=QByteArray::fromBase64(item["mediaContents"].toVariant().toByteArray());
        if(mediaContents.length())
        {
            QImage image;
            //image.load("/home/pierre/mur en pierre.JPG");
            image.loadFromData(mediaContents);
            QPixmap buffer      =QPixmap::fromImage(image);
            //i.icon              =QIcon(buffer);
            i.icon.addPixmap(buffer);
        }
        //TM_TRACE(TAG,QString("Item received: %1 (media content length: %2)").arg(i.title).arg(mediaContents.length()));
        _items.insert(id,i);
        emit newsUpdated(i);

    }
}

//
void                            Feed::onAutoReconnect()
{
    if(!_wsConnected)
    {
        connectFeed();
    }
}

//
void                            Feed::onProcessMessages()
{
    int messageProcessed=0;
    while( (_pendingMessages.size())&&(messageProcessed<MAX_MESSAGES_PER_SECOND) )
    {
        QJsonDocument           doc     =QJsonDocument::fromBinaryData(_pendingMessages.takeFirst());
        //qDebug() << doc.toJson();
        QJsonObject             event   =doc.object();
        const QString           type    =event["type"].toString();
        if(type=="update")
        {
            if(event["data"].isArray())
            {
                QJsonArray      news    =event["data"].toArray();
                for(int i=0;i<news.size();i++)
                {
                    updateNews(news.at(i).toObject());
                }
            }
            if(event["data"].isObject())
            {
                updateNews(event["data"].toObject());
            }

        }
        else if(type=="ready")
        {
            _ready      =true;
        }
        else
        {
            TM_WARNING(TAG,"Unkown event type: "+type);
        }
        messageProcessed++;
    }
}

//
void                            Feed::onWsConnected()
{
    TM_TRACE(TAG,"Socket connected");
    _wsConnected    =true;
    _wsConnecting   =false;
}

//
void                            Feed::onWsDisconnected()
{
    TM_TRACE(TAG,"Socket disconnected");
    _wsConnected    =false;
    _wsConnecting   =false;
}

//
void                            Feed::onWsError(QAbstractSocket::SocketError /*error*/)
{
    TM_TRACE(TAG,"Socket error");
    _wsConnecting   =false;
}

//
void                            Feed::onWsTextMessageReceived(QString message)
{
    // should not happen
    TM_TRACE(TAG,"TEXT MESSAGE RECEIVED");
    TM_TRACE(TAG,message);
    Q_ASSERT(0);
}

//
void                            Feed::onWsBinaryMessageReceived(const QByteArray& message)
{
    _pendingMessages.append(message);
}



TM_CORE_END()
