#include "Application.h"
#include "MainWindow.h"
#include <QLibraryInfo>
#include <QApplication>
#include <QStyleFactory>

#include <md/Asset.h>

//
#define TAG     "Application"

//
Application     Application::_instance;

//
Application&    Application::instance()
{
    return(_instance);
}

//
Application::Application()
{
    _settings=new Settings();
}

//
Application::~Application()
{
    if(_connection)
    {
        delete(_connection);
    }
    delete(_cmc);
    delete(_marketData);
    delete(_settings);
    delete(_assetsManager);
    if(_mainWindow)
    {
        _mainWindow->deleteLater();
        _mainWindow=nullptr;
    }
}

//
void                            Application::start()
{
}

//
rest::Connection&               Application::connection()
{
    if(!_connection)
    {
        _connection=new rest::Connection();
    }
    return(*_connection);
}

//
Settings&                       Application::settings()
{
    return(*_settings);
}

//
void                            Application::setTranslation(QString locale)
{
    QString qtLocale=QString("qt_%1").arg(locale);
    QString translationFile=QString(":/localization/editor_%1").arg(locale);

    if(_qtTranslator)
    {
        QApplication::instance()->removeTranslator(_qtTranslator);
        _qtTranslator=nullptr;
    }
    if(_appTranslator)
    {
        QApplication::instance()->removeTranslator(_appTranslator);
        _appTranslator=nullptr;
    }
    if(!locale.isNull())
    {
        /* QT */
        _qtTranslator=new QTranslator();
        if(!_qtTranslator->load(qtLocale,QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        {
            TM_ERROR(TAG,"Unable to load QT system translation files");
        }
        QApplication::instance()->installTranslator(_qtTranslator);
        /* App */
        _appTranslator=new QTranslator();
        if(!_appTranslator->load(translationFile))
        {
            TM_ERROR(TAG,"Unable to load translation files");
        }
        QApplication::instance()->installTranslator(_appTranslator);
    }
}

//
Style&                          Application::style()
{
    return(_style);
}

//
MainWindow*                     Application::mainWindow()
{
    if(!_mainWindow)
    {
        _mainWindow=new MainWindow();
    }
    return(_mainWindow);
}

//
QNetworkAccessManager*          Application::networkManager()
{
    if(!_networkManager)
    {
        _networkManager=new QNetworkAccessManager();
    }
    return(_networkManager);
}

//
CoinMarketCap*                  Application::cmc()
{
    if(!_cmc)
    {
        _cmc=new CoinMarketCap();
    }
    return(_cmc);
}

//
tmio::md::MarketData*           Application::marketData()
{
    if(!_marketData)
    {
        _marketData=new tmio::md::MarketData(_connection);
    }
    return(_marketData);
}

//
tmio::cm::Subscriber*           Application::communitySubscriber()
{
    if(!_communitySubscriber)
    {
        _communitySubscriber=new tmio::cm::Subscriber(_connection);
    }
    return(_communitySubscriber);
}

//
am::AssetsManager*              Application::assetsManager()
{
    return(_assetsManager);
}

//
nws::Feed*                      Application::newsFeed()
{
    if(!_newsFeed)
    {
        _newsFeed=new tmio::nws::Feed(_connection);
    }
    return(_newsFeed);
}

//
bool                            Application::attachAccount(QString accountID,QString accountKey)
{
    settings().setAccountID(accountID,accountKey);
    _assetsManager=new am::AssetsManager(marketData());
    return(true);
}

