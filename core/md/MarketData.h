#ifndef TMIOCORE_MARKETDATA_H
#define TMIOCORE_MARKETDATA_H
#include <QObject>
#include <QTimer>
#include <QtWebSockets/QWebSocket>
#include "md.h"
#include "../rest/Connection.h"

TM_CORE_BEGIN(md)

//
class   Exchange;
class   ExchangeFeed;
class   Asset;
class   Source;

//
class   MarketData : public QObject
{
    Q_OBJECT
public:
    explicit MarketData(rest::Connection*,QObject *parent = nullptr);

    rest::Connection*           connection();
    bool                        ready();
    ExchangeFeed*               exchangeFeed(const QString& name);
    int                         coinID(const QString& exchange,const QString& symbol);
    // exchanges
    QList<QString>              exchanges();
    Exchange*                   exchange(QString id);
    QString                     loopupExchangeAsset(QString exchange,QString symbol);
    bool                        favoriteTicker(const QString& exchange,const QString& ticker);
    bool                        setFavoriteTicker(const QString& exchange,const QString& ticker,bool favorite);
    bool                        favoriteAsset(const QString& assetID);
    bool                        setFavoriteAsset(const QString& assetID,bool favorite);
    QStringList                 watchAssets();
    bool                        watchAsset(const QString& assetID);
    bool                        setWatchAsset(const QString& assetID,bool favorite);
    // assets
    QStringList                 assets();
    QStringList                 fiatAssets();
    Asset*                      asset(QString id);
    bool                        saveAsset(QString id);
    Asset*                      createAsset(QString id);
    bool                        deleteAsset(QString id);
    // sources
    void                        loadSources();
    bool                        sourcesReady();
    Source*                     source(const QString& name);
    QStringList                 availableSources();

private:
    void                        connectFeed();
    bool                        updateAsset(QJsonObject);

signals:
    void                        assetModified(const QString& id);
    void                        assetDeleted(const QString& id);
    void                        favoriteTickerChanged(const QString& exchange,const QString& ticker,bool favorite);
    void                        favoriteAssetChanged(const QString& assetID,bool favorite);
    void                        watchAssetChanged(const QString& assetID,bool favorite);

public slots:
    void                        onCoinmapResult(tmio::rest::RequestResult&);
    void                        onExchangesResult(tmio::rest::RequestResult&);
    void                        onAssetsResult(tmio::rest::RequestResult&);
    void                        onTimer();
    void                        onWsConnected();
    void                        onWsDisconnected();
    void                        onWsError(QAbstractSocket::SocketError error);
    void                        onWsTextMessageReceived(QString message);
    void                        onWsBinaryMessageReceived(const QByteArray& message);

private:
    rest::Connection*           _connection;
    rest::Request*              _coinmapRequest     =nullptr;
    rest::Request*              _assetsRequest      =nullptr;
    rest::Request*              _exchangesRequest   =nullptr;
    QMap<QString,int>           _coinMap;
    QMap<QString,Asset*>        _assets;
    QMap<QString,Exchange*>     _exchanges;
    QVector<Source*>            _sources;
    QTimer*                     _timer;
    QWebSocket                  _webSocket;
    QUrl                        _wsUrl;
    bool                        _wsConnected        =false;
    bool                        _wsConnecting       =false;
    QMap<QString,QString>       _assetsLookup;
};

TM_CORE_END()
#endif // TMIOCORE_MARKETDATA_H
