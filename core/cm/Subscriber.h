#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include "cm.h"
#include <QMap>
#include <QTimer>
#include <QtWebSockets/QWebSocket>
#include "../rest/Connection.h"

TM_CORE_BEGIN(cm)

//
class Channel;

//
class Subscriber : public QObject
{
    Q_OBJECT
public:
    explicit Subscriber(rest::Connection* connection,QObject *parent = nullptr);

    void                        subscribe(const QString& channel);
    void                        unsubscribe(const QString& channel);

private:
    void                        sendSubscribeMessage(Channel* channel,bool subscribe);

signals:

public slots:
    void                        onTimer();
    void                        onWsConnected();
    void                        onWsDisconnected();
    void                        onWsError(QAbstractSocket::SocketError error);
    void                        onWsTextMessageReceived(QString message);
    void                        onWsBinaryMessageReceived(const QByteArray& message);

private:
    rest::Connection*           _connection;
    QMap<QString,Channel*>      _channels;
    QWebSocket                  _webSocket;
    QUrl                        _wsUrl;
    bool                        _wsConnected            =false;
    QTimer                      _timer;
};

TM_CORE_END()

#endif // SUBSCRIBER_H
