#pragma once
#include <rest/Connection.h>
#include <QTranslator>
#include <QApplication>
#include "libs/md/cmc/CoinMarketCap.h"
#include "md/MarketData.h"
#include "cm/Subscriber.h"
#include "am/AssetsManager.h"
#include "nws/Feed.h"
#include "Settings.h"
#include "Style.h"

#define PROP_TOOL_NAME              "tmToolName"
#define PROP_TOOL_ICON              "tmToolIcon"
#define PROP_TOOL_EXCHANGE_NAME     "tmExchange"
#define PROP_TOOL_TICKER_NAME       "tmTickerName"
#define PROP_TOOL_CMC_COIN_PAIR     "tmCmcCoinPair"
#define PROP_TOOL_ASSET_ID          "tmAssetID"
#define PROP_TOOL_API_ID            "tmApiID"
#define PROP_TOOL_API_KEY           "tmApiKey"
#define PROP_TOOL_API_ADDRESS       "tmApiAddress"
#define PROP_TOOL_URL               "tmURL"
#define PROP_TOOL_TITLE             "tmTitle"


//
using namespace tmio;

//
class MainWindow;

//
class Application
{
public:
    static Application&         instance();

    Application();
    ~Application();

    enum eColor
    {

    };

    void                        start();
    rest::Connection&           connection();
    QIcon                       avatarIcon();
    /* Settings                 */
    Settings&                   settings();
    void                        setTranslation(QString locale);
    /* Style                    */
    Style&                      style();
    /* Startup                  */

    /* Main window              */
    MainWindow*                 mainWindow();
    /* Network                  */
    QNetworkAccessManager*      networkManager();
    /* APIs                     */
    CoinMarketCap*              cmc();
    md::MarketData*             marketData();
    tmio::cm::Subscriber*       communitySubscriber();
    am::AssetsManager*          assetsManager();
    nws::Feed*                  newsFeed();
    bool                        attachAccount(QString accountID,QString accountKey);

private:
    static Application          _instance;
    rest::Connection*           _connection         =nullptr;
    Settings*                   _settings           =nullptr;
    QNetworkAccessManager*      _networkManager     =nullptr;
    MainWindow*                 _mainWindow         =nullptr;
    CoinMarketCap*              _cmc                =nullptr;
    md::MarketData*             _marketData         =nullptr;
    cm::Subscriber*             _communitySubscriber=nullptr;
    am::AssetsManager*          _assetsManager      =nullptr;
    nws::Feed*                  _newsFeed           =nullptr;
    Style                       _style;
    QIcon                       _avatarIcon;
    //api::ExchangeFeed

signals:
    //void                        sigLog(eg::Core::eLogType,QString text);
    QTranslator*                _qtTranslator       =nullptr;
    QTranslator*                _appTranslator      =nullptr;
};
