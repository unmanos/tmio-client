#ifndef TMIOCORE_NWS_FEED_H
#define TMIOCORE_NWS_FEED_H
#include "nws.h"
#include <QMap>
#include <QTimer>
#include <QList>
#include <QIcon>
#include <QtWebSockets/QWebSocket>
#include "../rest/Connection.h"

TM_CORE_BEGIN(nws)

//
class Feed : public QObject
{
    Q_OBJECT
public:
    explicit Feed(rest::Connection* connection,QObject *parent = nullptr);

    struct                  Item
    {
        QString             feedTitle;
        QString             title;
        QString             description;
        QDateTime           pubDate;
        QIcon               icon;
        QString             link;
    };

    QList<Item>                 items();

private:
    void                        connectFeed();
    void                        updateNews(QJsonObject);

signals:
    void                        newsUpdated(const Item& i);

public slots:
    void                        onAutoReconnect();
    void                        onProcessMessages();
    void                        onWsConnected();
    void                        onWsDisconnected();
    void                        onWsError(QAbstractSocket::SocketError error);
    void                        onWsTextMessageReceived(QString message);
    void                        onWsBinaryMessageReceived(const QByteArray& message);

private:
    rest::Connection*           _connection;
    QWebSocket                  _webSocket;
    QUrl                        _wsUrl;
    bool                        _wsConnected            =false;
    bool                        _wsConnecting           =false;
    QTimer                      _autoReconnectTimer;
    QTimer                      _processMessagesTimer;
    QMap<QString,Item>          _items;
    QList<QByteArray>           _pendingMessages;
    bool                        _ready                  =false;
};

TM_CORE_END()

#endif // TMIOCORE_NWS_FEED_H
