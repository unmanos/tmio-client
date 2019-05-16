#include "MdiArea.h"
#include "ui_MdiArea.h"
#include "Application.h"
#include <QMdiSubWindow>
#include <QMetaProperty>
#include <md/Asset.h>
#include "forms/views/tickers/TickerPriceView.h"
#include "forms/views/coins/CoinMarketCapView.h"
#include "forms/views/mining/NicehashView.h"
#include "forms/views/assets/AssetsSummaryView.h"
#include "forms/views/markets/ExchangeLiveView.h"
#include "forms/views/assets/MarketDataAssetsView.h"
#include "forms/views/assets/AssetView.h"
#include "forms/views/browser/BrowserView.h"
#include "forms/views/admin/AccountsManagerView.h"
#include "forms/views/dashboard/DashboardView.h"
#include "forms/views/explorers/BitcoinBlockExplorerView.h"

//
#define TAG                    "MdiArea"
#define VIEW_ID_PROP           "tmioViewID"

//
MdiArea::MdiArea(QMenu* menu,QWidget *parent) :
    QMdiArea(parent),
    ui(new Ui::MdiArea)
{
    ui->setupUi(this);

    // http://lists.qt-project.org/pipermail/development/2014-November/019093.html


    //setDocumentMode(true);
    setViewMode(QMdiArea::TabbedView);
    //setDocumentMode(true);
    setTabsClosable(true);
    setTabShape(QTabWidget::Rounded);
    //setTabShape(QTabWidget::Triangular);
    setTabsMovable(true);
    //setTa

    _viewMenu=menu;

    connect(this,SIGNAL(subWindowActivated(QMdiSubWindow*)),this,SLOT(onSubWindowActivated(QMdiSubWindow*)));
}

//
MdiArea::~MdiArea()
{
    delete ui;
}

//
bool        MdiArea::activateView(QString viewID)
{
    QList<QMdiSubWindow*>       windows=subWindowList();
    for(QList<QMdiSubWindow*>::iterator i=windows.begin();i!=windows.end();++i)
    {
        if((*i)->property(VIEW_ID_PROP)==viewID)
        {
            //(*i)->activateWindow();
            setActiveSubWindow(*i);
            return(true);
        }
    }
    return(false);
}

//
/*
static void dump_props(QObject *o)
{
  auto mo = o->metaObject();
  qDebug() << "## Properties of" << o << "##";
  do {
    qDebug() << "### Class" << mo->className() << "###";
    std::vector<std::pair<QString, QVariant> > v;
    v.reserve(mo->propertyCount() - mo->propertyOffset());
    for (int i = mo->propertyOffset(); i < mo->propertyCount();
          ++i)
      v.emplace_back(mo->property(i).name(),
                     mo->property(i).read(o));
    std::sort(v.begin(), v.end());
    for (auto &i : v)
      qDebug() << i.first << "=>" << i.second;
  } while ((mo = mo->superClass()));
}
*/

//
bool        MdiArea::createView(QAction* action)
{
    QString         toolName    =action->property(PROP_TOOL_NAME).toString();
    QString         toolIcon    =action->property(PROP_TOOL_ICON).toString();
    QString         viewID      =toolName+"+";
    bool            found       =false;
    QWidget*        view        =nullptr;
    QMdiSubWindow*  win         =nullptr;
    if(toolName=="exchange/openticker")
    {
        // TickerPriceView
        QString                  tickerName=action->property(PROP_TOOL_TICKER_NAME).toString();
        QString                  assetID=action->property(PROP_TOOL_ASSET_ID).toString();
        QString                  exchange=action->property(PROP_TOOL_EXCHANGE_NAME).toString();
        viewID+=QString(exchange)+"+";
        viewID+=QString(tickerName)+"+";
        viewID+=assetID;
        if(!activateView(viewID))
        {
            TM_TRACE(TAG,QString("Requesting '%1' on ticker '%2'").arg(toolName,tickerName));
            view=new TickerPriceView(exchange,assetID,tickerName,this);
            //view->setWindowTitle(QString("Ticker %1").arg(tickerName));

            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":icons/exchanges/"+exchange));

            //view->setProperty(PROP_UTILITY_ID,utilityID);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
        }
        found=true;
    }
    if(toolName=="coinmarketcap/open")
    {
        if(!activateView(viewID))
        {
            view=new CoinMarketCapView();
            view->setWindowTitle(QString("Market capitalization"));
            //view->setProperty(PROP_UTILITY_ID,utilityID);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":/icons/marketdata.png"));
        }
        found=true;
    }
    if(toolName=="mining/nicehash")
    {
        QString                  apiID      =action->property(PROP_TOOL_API_ID).toString();
        QString                  apiAddress =action->property(PROP_TOOL_API_ADDRESS).toString();
        viewID+=QString(apiAddress)+"+";
        if(!activateView(viewID))
        {
            view=new NicehashView(apiAddress);
            view->setWindowTitle(QString("Nicehash"));
            //view->setProperty(PROP_UTILITY_ID,utilityID);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":/icons/nicehash.png"));
        }
        found=true;
    }
    if(toolName=="assets/summary")
    {
        if(!activateView(viewID))
        {
            view=new AssetsSummaryView();
            view->setWindowTitle(QString("Assets summary"));
            //view->setProperty(PROP_UTILITY_ID,utilityID);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":/icons/assets.png"));
        }
        found=true;
    }
    if(toolName=="markets/liveview")
    {
        QString                  exchange=action->property(PROP_TOOL_EXCHANGE_NAME).toString();
        viewID+=QString(exchange)+"+";
        if(!activateView(viewID))
        {
            view=new ExchangeLiveView(tmio::md::exchangeBinance);
            view->setWindowTitle(QString("Live view"));
            //view->setProperty(PROP_UTILITY_ID,utilityID);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            //win->setWindowIcon(QIcon(":/icons/binance.png"));
        }
        found=true;
    }
    if(toolName=="marketdata/assets")
    {
        if(!activateView(viewID))
        {
            view=new MarketDataAssetsView();
            view->setWindowTitle(QString("Available assets"));
            //view->setProperty(PROP_UTILITY_ID,utilityID);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":/icons/marketdata.png"));
        }
        found=true;
    }
    if(toolName=="asset")
    {
        QString                  assetID=action->property(PROP_TOOL_ASSET_ID).toString();
        md::Asset*               asset  =Application::instance().marketData()->asset(assetID);
        if(asset)
        {
            viewID+=assetID;
            if(!activateView(viewID))
            {
                view=new AssetView(assetID);
                view->setWindowTitle(asset->name());
                //view->setProperty(PROP_UTILITY_NAME,utilityName);
                win=addSubWindow(view);
                win->setProperty(VIEW_ID_PROP,viewID);
                win->showMaximized();
                win->setAttribute(Qt::WA_DeleteOnClose);
                win->setWindowIcon(Application::instance().cmc()->assetIcon(assetID));
            }
            found=true;
        }
        TM_ERROR(TAG,"Unknown asset "+assetID);
    }
    //
    if(toolName=="dashboard")
    {
        if(!activateView(viewID))
        {
            view=new DashboardView();
            view->setWindowTitle("Dashboard");
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":/icons/dashboard.png"));
            //win->setWindowIcon(Application::instance().cmc()->assetIcon(assetID));
        }
        found=true;
    }
    //
    if(toolName=="explorers/bitcoin")
    {
        if(!activateView(viewID))
        {
            view=new BitcoinBlockExplorerView();
            view->setWindowTitle("Bitcoin block explorer");
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            win->setWindowIcon(QIcon(":/icons/common/blocks.png"));
            //win->setWindowIcon(Application::instance().cmc()->assetIcon(assetID));
        }
        found=true;
    }
    //
    if(toolName=="browser")
    {
        QString                  url    =action->property(PROP_TOOL_URL).toString();
        QString                  title  =action->property(PROP_TOOL_TITLE).toString();
        if(!activateView(viewID))
        {
            view=new BrowserView(title,url);
            view->setWindowTitle(title);
            //view->setProperty(PROP_UTILITY_NAME,utilityName);
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setAttribute(Qt::WA_DeleteOnClose);
            //win->setWindowIcon(QIcon(":/icons/coinmarketcap.png"));
            //win->setWindowIcon(Application::instance().cmc()->assetIcon(assetID));
        }
        found=true;
    }
    //
    if(toolName=="admin/accounts")
    {
        if(!activateView(viewID))
        {
            view=new AccountsManagerView();
            view->setWindowTitle("Account");
            win=addSubWindow(view);
            win->setProperty(VIEW_ID_PROP,viewID);
            win->showMaximized();
            win->setWindowIcon(QIcon(":/icons/admin/accounts.png"));
            win->setAttribute(Qt::WA_DeleteOnClose);
        }
        found=true;
    }
    //
    if(view&&win)
    {
        bool    menuExist=false;
        foreach(QAction* menuAction,_viewMenu->actions())
        {
            if(menuAction->property(VIEW_ID_PROP).toString()==viewID)
                menuExist=true;
        }
        if(!menuExist)
        {
            // add to menu
            QAction*     menuAction=new QAction();
            QList<QByteArray>       propertyNames=action->dynamicPropertyNames();
            foreach(QByteArray name,propertyNames)
            {
                menuAction->setProperty(name,action->property(name));
            }
            menuAction->setText(view->windowTitle());
            menuAction->setIcon(win->windowIcon());
            menuAction->setProperty(VIEW_ID_PROP,viewID);
            _viewMenu->addAction(menuAction);
            connect(menuAction,SIGNAL(triggered()),this,SLOT(onMenuAction()));
        }
    }
    //
    return(found);
}

//
void        MdiArea::onSubWindowActivated(QMdiSubWindow* subWindow)
{
    TM_TRACE(TAG,"Sub window activated");
    if(subWindow)
    {
        //emit sigOnViewActivated(subWindow->property(PROP_UTILITY_ID).toString());
    }
    else
    {
        TM_TRACE(TAG,"No more window active");
    }
}

//
void        MdiArea::onMenuAction()
{
    QAction*        menuAction=dynamic_cast<QAction*>(sender());
    //activateView(menuAction->property(VIEW_ID_PROP).toString());
    createView(menuAction);
}
