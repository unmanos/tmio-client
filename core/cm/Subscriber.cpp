#include "Subscriber.h"
#include "Channel.h"
TM_CORE_BEGIN(cm)

#define TAG                     "cm::Subscriber"
#define WS_URL                  "community"

//
Subscriber::Subscriber(rest::Connection* connection,QObject *parent) : QObject(parent)
{
    _connection             =connection;
    // websocket for real time updates
    _wsUrl=QUrl(QString("%1/%2?token=%3").arg(connection->wsRoot()).arg(WS_URL).arg(connection->token()));
    connect(&_webSocket, &QWebSocket::connected, this, &Subscriber::onWsConnected);
    connect(&_webSocket, &QWebSocket::disconnected, this, &Subscriber::onWsDisconnected);
    connect(&_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&_webSocket, &QWebSocket::textMessageReceived,
            this, &Subscriber::onWsTextMessageReceived);
    connect(&_webSocket, &QWebSocket::binaryMessageReceived,
            this, &Subscriber::onWsBinaryMessageReceived);
    _webSocket.open(QUrl(_wsUrl));
    // timer
    connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    _timer.start(1000*10);
}

//
void                            Subscriber::subscribe(const QString& channel)
{

    if(!_channels.contains(channel))
    {
        Channel*        c=new Channel(this,channel);
        _channels.insert(channel,c);
        // send register message
        sendSubscribeMessage(c,true);
    }
    else
    {
        TM_WARNING(TAG,"Subscribe ignored, already subscribed to "+channel);
    }
}

//
void                            Subscriber::unsubscribe(const QString& channel)
{
    if(_channels.contains(channel))
    {
        sendSubscribeMessage(_channels[channel],false);
        delete(_channels[channel]);
        _channels.remove(channel);
    }
    else
    {
        TM_WARNING(TAG,"Unsubscribe ignored, not subscribed to "+channel);
    }
}

//
void                            Subscriber::sendSubscribeMessage(Channel* channel,bool subscribe)
{
    QJsonObject     message;
    message.insert("type",subscribe?"subscribe":"unsubscribe");
    message.insert("channel",channel->name());
    _webSocket.sendBinaryMessage(QJsonDocument(message).toBinaryData());
    TM_TRACE(TAG,QString("%1 to channel %2").arg(subscribe?"Subscribe":"Unsubscribe").arg(channel->name()));
}

//
void                            Subscriber::onTimer()
{
    if(!_wsConnected)
    {
        TM_TRACE(TAG,"socket not connected, connecting.");
        TM_TRACE(TAG,_wsUrl.toString());
        _webSocket.open(QUrl(_wsUrl));
    }
}

//
void                            Subscriber::onWsConnected()
{
    TM_TRACE(TAG,"Socket connected");
    _wsConnected    =true;
    foreach(Channel* c,_channels)
    {
        sendSubscribeMessage(c,true);
    }

}

//
void                            Subscriber::onWsDisconnected()
{
    TM_TRACE(TAG,"Socket disconnected");
    _wsConnected    =false;
}

//
void                            Subscriber::onWsError(QAbstractSocket::SocketError /*error*/)
{
    TM_TRACE(TAG,"Socket error");
}

//
void                            Subscriber::onWsTextMessageReceived(QString message)
{
    // should never happen
    TM_TRACE(TAG,"TEXT MESSAGE RECEIVED");
    TM_TRACE(TAG,message);
    Q_ASSERT(0);
}

//
void                            Subscriber::onWsBinaryMessageReceived(const QByteArray& message)
{
    TM_TRACE(TAG,"BINARY MESSAGE RECEIVED");
    QJsonObject             event=QJsonDocument::fromJson(message).object();
    // TODO
}



TM_CORE_END()
