#ifndef TMIOCORE_EXCHANGEFEED_H
#define TMIOCORE_EXCHANGEFEED_H
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QMap>
#include <QTimer>
#include "md.h"
#include "../rest/Connection.h"
#include "../rest/Request.h"
TM_CORE_BEGIN(md)

//
class MarketData;

//
class ExchangeFeed : public QObject
{
    Q_OBJECT
public:
    explicit ExchangeFeed(rest::Connection* connection,const QString& name,MarketData* marketData);

    struct      Ticker
    {
        QJsonObject         json;
        QString             name;
        QDateTime           time;
        double              price;
        QMap<int,double>    previousPrices;
        QMap<int,double>    quoteVolumes;
        QMap<int,double>    volumes;
        //int                 cmcId;
        QString             assetID;
        QString             quoteID;
        //QString     symbol;
        //QString     currency;
    };

    bool                    ready();
    const QString&          name();
    QVector<Ticker>         tickers();
    Ticker                  ticker(const QString& name);

private:
    void                    connectFeed();
    void                    updateTicker(const QJsonObject&);


signals:
    //void                  closed();
    void                    tickerChanged(const tmio::md::ExchangeFeed::Ticker&);

public slots:
    void                    onAutoReconnect();
    void                    onConnected();
    void                    onDisconnected();
    void                    onError(QAbstractSocket::SocketError error);
    void                    onTextMessageReceived(QString message);
    void                    onBinaryMessageReceived(const QByteArray& message);
    void                    onReceiveTickers(tmio::rest::RequestResult& result);

private:
    MarketData*             _marketData     =nullptr;
    QString                 _name;
    tmio::rest::Request*    _tickerRequest;
    QWebSocket*             _webSocket;
    QUrl                    _wsUrl;
    bool                    _debug;
    QMap<QString,Ticker>    _tickers;
    QTimer                  _autoReconnectTimer;
    bool                    _connected      =false;
    bool                    _ready          =false;
    QMap<quint16,QString>   _shortIdLookup;
    bool                    _connecting     =false;
    QDateTime               _wsReceivedTime;
};

TM_CORE_END()

#endif // TMIOCORE_EXCHANGEFEED_H
