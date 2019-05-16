#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Application.h"
#include "forms/widgets/ExchangeFeedsWidget.h"
#include "forms/widgets/CoinInfoWidget.h"
#include "forms/widgets/CommunityWidget.h"
#include "forms/widgets/AssetWidget.h"
#include "forms/widgets/NewsFeedWidget.h"
#include "forms/widgets/ExchangeFeedsWidget.h"
#include "forms/views/MdiArea.h"
#include "forms/system/UserProfileDialog.h"
#include "forms/system/AboutDialog.h"
#include "forms/wallets/ConfigureWalletsDialog.h"
#include "libs/ui/style/IconnedDockStyle.h"
#include <rest/Connection.h>
#include <rest/Request.h>
#include <api/Account.h>
#include <md/MarketData.h>
#include <md/Asset.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include <QDockWidget>
#include <QListWidget>
#include <QScrollBar>
#include <QTimer>
#include <QTimeZone>

#define TAG         "MainWindow"

//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    _logsWidget=new QTreeView();
    _logsModel=new QStandardItemModel();
    _logsWidget->setModel(_logsModel);
    _logsWidget->setHeaderHidden(true);
    //_logsWidget->setReadOnly(true);
    _logsWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->setupUi(this);

    _exchangeFeedsWidget=new ExchangeFeedsWidget();
    QDockWidget *dockWidget = new QDockWidget("Market data");
    dockWidget->setMinimumWidth(384);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    dockWidget->setWidget(_exchangeFeedsWidget);
    connect(_exchangeFeedsWidget,SIGNAL(sigActionTriggered(QAction*)),SLOT(onToolboxAction(QAction*)));

    _mdiArea= new MdiArea(ui->menuViews);
    setCentralWidget(_mdiArea);
    connect(_mdiArea,SIGNAL(sigOnViewActivated(QString)),this,SLOT(onMdiViewActivated(QString)));

    connect(&tmio::Core::instance(),SIGNAL(sigLog(tmio::Core::eLogType,QString)),this,SLOT(onLog(tmio::Core::eLogType,QString)));

    statusBar()->showMessage(tr("You are now connected to the TokenMark.IO platform."));
    statusBar()->hide();

    buildToolbar();

    setDockNestingEnabled(true);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

        _dockProperties= new QDockWidget("Properties");
        _dockProperties->setMinimumWidth(256);
        addDockWidget(Qt::LeftDockWidgetArea, _dockProperties);
        _dockProperties->setStyle(new IconnedDockStyle( QIcon(":/icons/common/info.png"), nullptr));

        // Third in tab with second one
        _holdingsWidget=new AssetsHoldingsWidget();
        QDockWidget *dockWidgetHoldings = new QDockWidget("Holdings");
        //dockWidgetHoldings->setMinimumWidth(400);
        //dockWidgetHoldings->resize(400,10);
        //dockWidgetHoldings->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        addDockWidget(Qt::BottomDockWidgetArea, dockWidgetHoldings);
        dockWidgetHoldings->setWidget(_holdingsWidget);
        dockWidgetHoldings->setStyle(new IconnedDockStyle( QIcon(":/icons/assets.png"), nullptr));
        //dockWidgetHoldings


        // Third in tab with second one
        QDockWidget *dockWidgetTraces = new QDockWidget("Traces");
        //dockWidgetTraces->setMinimumHeight(256);
        //dockWidgetTraces->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        addDockWidget(Qt::BottomDockWidgetArea, dockWidgetTraces);
        dockWidgetTraces->setStyle(new IconnedDockStyle( QIcon(":/icons/common/sticky.png"), nullptr));
        dockWidgetTraces->setWidget(_logsWidget);

        // Third in tab with second one
        /*_communityWidget=new CommunityWidget();
        QDockWidget *dockWidgetCommunity = new QDockWidget("Community");
        //dockWidgetCommunity->setMinimumHeight(256);
        //dockWidgetCommunity->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        addDockWidget(Qt::BottomDockWidgetArea, dockWidgetCommunity);
        dockWidgetCommunity->setWidget(_communityWidget);
        tabifyDockWidget(dockWidgetCommunity, dockWidgetTraces);
        //dockWidgetCommunity->raise();*/

        // Third in tab with second one
        _newsFeedWidget=new NewsFeedWidget();
        QDockWidget *dockWidgetNewsFeed = new QDockWidget("News feed");
        //dockWidgetNewsFeed->setMinimumHeight(256);
        //dockWidgetNewsFeed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        addDockWidget(Qt::BottomDockWidgetArea, dockWidgetNewsFeed);
        dockWidgetNewsFeed->setWidget(_newsFeedWidget);
        tabifyDockWidget(dockWidgetTraces, dockWidgetNewsFeed);
        dockWidgetNewsFeed->raise();

        // Quad in tab with second one
        //_assetWidget=new CommunityWidget();
        /*QDockWidget *dockWidgetCommunity = new QDockWidget("Community");
        dockWidgetCommunity->setMinimumHeight(256);
        addDockWidget(Qt::BottomDockWidgetArea, dockWidgetCommunity);
        dockWidgetCommunity->setWidget(_communityWidget);
        tabifyDockWidget(dockWidgetCommunity, dockWidgetTraces);
        dockWidgetCommunity->raise();*/



    //dockWidgetHoldings->resize(100,10);
    //_holdingsWidget->resize(100,10);
    //_newsFeedWidget->resize(1200,10);
    _holdingsWidget->setMinimumWidth(300);
    _newsFeedWidget->setMinimumWidth(500);

    // connect menu actions
    connect(ui->actionMenuAbout, SIGNAL(triggered()), this, SLOT(onActionMenuAbout()));
    connect(ui->actionMenuExit, SIGNAL(triggered()), this, SLOT(onActionMenuExit()));
    connect(ui->actionMenuMiningNicehash, SIGNAL(triggered()), this, SLOT(onActionMenuMiningNicehash()));
    connect(ui->actionMenuAdminAccounts, SIGNAL(triggered()), this, SLOT(onActionMenuAdminAccounts()));
    //connect(ui->actionExchangeConfigureApiKeys, SIGNAL(triggered()), this, SLOT(onActionConfigureEchangeKeys()));
    //connect(ui->actionAssetsSummary, SIGNAL(triggered(bool)), this, SLOT(onActionAssetsSummary()));
    connect(ui->actionExchangeBinanceLiveView, SIGNAL(triggered(bool)), this, SLOT(onActionExchangeBinanceLiveView()));
    connect(ui->actionWalletsConfigure, SIGNAL(triggered(bool)), this, SLOT(onActionWalletsConfigure()));
    connect(ui->actionWalletsBalance, SIGNAL(triggered(bool)), this, SLOT(onActionAssetsSummary()));
    connect(ui->actionMenuViewsDashboard, SIGNAL(triggered(bool)), this, SLOT(onActionDashboard()));
    connect(ui->actionBitcoinBlockExplorer, SIGNAL(triggered(bool)), this, SLOT(onActionBitcoinBlockExplorer()));

    connect(Application::instance().marketData(),&tmio::md::MarketData::watchAssetChanged,this,&MainWindow::onWatchAssetChanged);
    // remove admin menu
    if(!Application::instance().connection().account()->isMemberOf("admin"))
    {
        delete(ui->menuAdmin);
    }
    #ifndef QT_DEBUG
    delete(ui->menuMining);
    delete(ui->menuExplorers);
    #endif

    TM_INFO(TAG,"Update asset manager");
    Application::instance().settings().updateAssetsManager(Application::instance().assetsManager());

    openAssetProperties(md::Asset::BTC);
    #ifdef QT_DEBUG
    onActionDashboard();
    #endif
    TM_INFO(TAG,"Application main window created");


}

//
MainWindow::~MainWindow()
{
    delete ui;
}

//
void                MainWindow::createMdiView(QAction* action)
{
    if(_mdiArea->createView(action))
    {
        TM_TRACE(TAG,"MDI handled the tool");
    }
    else
    {
        TM_ERROR(TAG,"Action triggered from ToolBox have no handler");
    }
}

//
void                MainWindow::openAssetProperties(const QString& assetID)
{
    if(_currentAssetProperties!=assetID)
    {
        {
            if(_dockProperties->widget())
            {
                delete(_dockProperties->widget());
            }
            CoinInfoWidget*         assetInfoWidget=new CoinInfoWidget(assetID,_dockProperties);
            _dockProperties->setWidget(assetInfoWidget);
            connect(assetInfoWidget,SIGNAL(sigActionTriggered(QAction*)),SLOT(onToolboxAction(QAction*)));
        }
        //
        if(_communityWidget)
        {
            _communityWidget->openAsset(assetID);
        }
        _currentAssetProperties=assetID;
    }
}

//
QString             MainWindow::currentAssetProperties()
{
    return(_currentAssetProperties);
}

//
void                MainWindow::openBrowserURL(const QString& url,const QString& title)
{
    TM_TRACE(TAG,"onActionMenuMiningNicehash");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"browser");
    action.setProperty(PROP_TOOL_URL,url);
    action.setProperty(PROP_TOOL_TITLE,title);
    _mdiArea->createView(&action);
}

//
/*void                MainWindow::openAssetCommunity(const QString& assetID)
{
    _communityWidget->openAsset(assetID);
}*/

//
void                MainWindow::buildToolbar()
{
    CoinMarketCap*          cmc         =Application::instance().cmc();
    md::MarketData*         marketData  =Application::instance().marketData();
    int                     fontSize    =11;
    tmio::rest::Connection& conn        =Application::instance().connection();
    ui->mainToolBar->setWindowTitle("Tool bar");
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

    //ui->mainToolBar->setOrientation(Qt::Horizontal);
    ui->mainToolBar->setAllowedAreas(Qt::ToolBarArea::TopToolBarArea);
    /* Timer        */
    {
        QTimer*     timer=new QTimer(this);
        connect(timer,SIGNAL(timeout()),this,SLOT(onRefreshToolbar()));
        timer->start(1000);
    }
    /* App icon */
    /*{
        QLabel*     icon=new QLabel(this);
        icon->setPixmap(QIcon(":/icons/app.png").pixmap(20,20));
        ui->mainToolBar->addWidget(icon);
        ui->mainToolBar->addSeparator();
    }*/
    /* Intl. Clock */
    {
        //QLabel* label=new QLabel(this);
        //label->setText(tr("Time"));
        _clockLabel=new QLabel("00:00:00");
        QFont   font=_clockLabel->font();
        font.setBold(true);
        font.setPointSize(fontSize);
        _clockLabel->setFont(font);

        //ui->mainToolBar->addWidget(label);
        ui->mainToolBar->addWidget(_clockLabel);
        ui->mainToolBar->addSeparator();
        //_utilityCombo->hi
    }
    /* Fiat */
    {
        _quoteAssetCombo = new QComboBox(this);
        md::Asset*              bitcoin    =marketData->asset(md::Asset::BTC);
        md::Asset*              ethereum   =marketData->asset(md::Asset::ETH);
        _quoteAssetCombo->addItem(Application::instance().cmc()->assetIcon(bitcoin->id()),bitcoin->symbol(),bitcoin->id());
        _quoteAssetCombo->addItem(Application::instance().cmc()->assetIcon(ethereum->id()),ethereum->symbol(),ethereum->id());
        foreach(QString fiatID,marketData->fiatAssets())
        {
            md::Asset*              fiat    =marketData->asset(fiatID);
            _quoteAssetCombo->addItem(cmc->assetIcon(fiatID),fiat->symbol(),fiatID);

        }
        _quoteAssetCombo->setCurrentText(cmc->quoteAssetSymbol());
        ui->mainToolBar->addWidget(_quoteAssetCombo);
        ui->mainToolBar->addSeparator();
        connect(_quoteAssetCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onRefreshToolbar()));
    }

    _actionWatchDummy=new QAction();
    ui->mainToolBar->addAction(_actionWatchDummy);
    _actionWatchDummy->setVisible(false);

    /* Total market cap     */
    {
        _marketDataAssets=new QToolButton();
        _marketDataAssets->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
        _marketDataAssets->setText("");
        QFont   font=_marketDataAssets->font();
        font.setBold(true);
        font.setPointSize(fontSize);
        _marketDataAssets->setFont(font);

        //_marketCap->setIconSize(QSize(5,5));
        _marketDataAssets->setIcon(QIcon(":/icons/app.png"));
        //_marketCap->resize(1,20);
        //_marketCap->setFixedHeight(22);
        ui->mainToolBar->addWidget(_marketDataAssets);
        ui->mainToolBar->addSeparator();
        connect(_marketDataAssets,SIGNAL(clicked(bool)),this,SLOT(onOpenMarketDataAssets()));
    }
    /* Spacer */
    {
        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // toolBar is a pointer to an existing toolbar
        ui->mainToolBar->addWidget(spacer);
        //toolBar->addAction("Right-aligned button");
    }
    /* Account */
    {
        connect(conn.account(),SIGNAL(changed()),this,SLOT(onAccountChanged()));
        _actionAccount=new QAction(this);
        ui->mainToolBar->addAction(_actionAccount);
        connect(_actionAccount,SIGNAL(triggered(bool)),this,SLOT(onOpenUserProfile()));
    }
    buildToolbarWatches();
    onAccountChanged();
    onRefreshToolbar();
}

//
void                MainWindow::buildToolbarWatches()
{
    TM_TRACE(TAG,"buildToolbarWatches");
    CoinMarketCap*  cmc     =Application::instance().cmc();
    md::MarketData* md      =Application::instance().marketData();
    const int               fontSize    =11;
    /* clean up old watch */
    foreach(WatchAsset wa,_watchAssets)
    {
        delete(wa.action);
    }
    //_watchToolbar->clear();
    _watchAssets.clear();
    QStringList                     watches=Application::instance().marketData()->watchAssets();
    // order watch by market cap
    QMap<double,QString>            rankSort;
    foreach(QString assetID,watches)
    {
        md::Asset*      asset=md->asset(assetID);
        double supply   =cmc->circulatingSupply(assetID);
        double price    =1;
        if( (supply>0.0001)&&(cmc->assetPrice(cmc->quoteAssetID(),asset->id(),price,nullptr)) )
        {
            double      cap=supply*price;
            rankSort.insert(cap,assetID);
        }
    }
    QStringList                     orderedWatches;
    foreach(QString assetID,rankSort.values())
    {
        orderedWatches.append(assetID);
    }
    foreach(QString assetID,watches)
    {
        if(!orderedWatches.contains(assetID))
            orderedWatches.insert(0,assetID);
    }
    for(int iWatch=0;iWatch<orderedWatches.length();iWatch++)
    {
        WatchAsset      wa;
        md::Asset*      asset=md->asset(orderedWatches.at(orderedWatches.length()-(iWatch+1)));
        wa.assetID  =asset->id();

        wa.action=new QAction();
        wa.action->setToolTip(asset->name());
        wa.action->setText("");
        QFont   font=_marketDataAssets->font();
        font.setBold(true);
        font.setPointSize(fontSize);
        wa.action->setFont(font);
        if(!cmc->assetIcon(wa.assetID).isNull())
        {
            wa.action->setIcon(cmc->assetIcon(wa.assetID));
            wa.iconFound=true;
        }
        //ui->mainToolBar->addAction(wa.action);
        ui->mainToolBar->insertAction(_actionWatchDummy,wa.action);

        _watchAssets.append(wa);
        connect(wa.action,SIGNAL(triggered(bool)),this,SLOT(onOpenWatchAsset()));
    }
}

//
void                MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    _logsWidget=nullptr;
}

//
void                MainWindow::onCurrentAccountResult(tmio::rest::RequestResult& result)
{
    QJsonDocument doc=result._document;
    TM_TRACE(TAG,"Account information loaded");
    QJsonObject object=doc.object();
}

//
void                MainWindow::onActionMenuAbout()
{
    TM_TRACE(TAG,"onActionMenuAbout");
    AboutDialog     dlg(this);
    dlg.setModal(true);
    dlg.exec();
}

//
void                MainWindow::onActionMenuExit()
{
    //close();
    _logsWidget=nullptr;
    QApplication::exit();
}

//
void                MainWindow::onActionMenuAdminAccounts()
{
    TM_TRACE(TAG,"Admin platform accounts");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"admin/accounts");
    _mdiArea->createView(&action);
}

//
void                MainWindow::onActionAssetsSummary()
{
    TM_TRACE(TAG,"onActionAssetsSummary");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"assets/summary");
    _mdiArea->createView(&action);
}

//
void                MainWindow::onActionDashboard()
{
    TM_TRACE(TAG,"onActionDashboard");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"dashboard");
    _mdiArea->createView(&action);
}

//
void                MainWindow::onActionBitcoinBlockExplorer()
{
    TM_TRACE(TAG,"onActionBitcoinBlockExplorer");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"explorers/bitcoin");
    _mdiArea->createView(&action);
}

//
void                MainWindow::onActionExchangeBinanceLiveView()
{
    TM_TRACE(TAG,"onActionExchangeBinanceLiveView");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"markets/liveview");
    action.setProperty(PROP_TOOL_EXCHANGE_NAME,md::exchangeBinance);
    _mdiArea->createView(&action);
}

//
void                MainWindow::onActionWalletsConfigure()
{
    TM_TRACE(TAG,"onActionWalletsConfigure");
    ConfigureWalletsDialog dlg(this);
    dlg.setModal(true);
    if(dlg.exec()==QDialog::Accepted)
    {
        Application::instance().settings().updateAssetsManager(Application::instance().assetsManager());
    }

}

//
void                MainWindow::onLog(tmio::Core::eLogType type,QString text)
{
    if(!_logsWidget)
        return;
    const Style&    style=Application::instance().style();
    const int       maxVisibleRows=100;
    QColor          color(0,0,0);
    switch(type)
    {
        case tmio::Core::logInfo:
        color=style.color(Style::colorTextInfo);
        break;
        case tmio::Core::logTrace:
        color=style.color(Style::colorTextTrace);
        break;
        case tmio::Core::logWarning:
        color=style.color(Style::colorTextWarning);
        break;
        case tmio::Core::logError:
        color=style.color(Style::colorTextError);
        break;
    }
    if(_logsWidget)
    {
        int         count=_logsModel->rowCount();
        bool        scroll=true;
        if(count)
        {
            QStandardItem*  previous=_logsModel->item(count-1);
            QRect           visualRect=_logsWidget->visualRect(previous->index());
            //qDebug() << visualRect;
            if(visualRect.bottom()>_logsWidget->height())
            {
                scroll=false;
            }
            if( ((count>(maxVisibleRows*2))&&(scroll))||(count>(maxVisibleRows*5)) )
            {
                _logsModel->removeRows(0,count-maxVisibleRows);
            }
        }
        QStandardItem*      item=new QStandardItem(text);
        item->setData(color,Qt::ForegroundRole);
        _logsModel->appendRow(item);
        if(scroll)
        {
            _logsWidget->scrollToBottom();
        }
    }
}

//
void                MainWindow::onMdiViewActivated(QString utilityID)
{
    TM_TRACE(TAG,QString("MDI view selected on utility %1").arg(utilityID));
}

//
void                MainWindow::onToolboxAction(QAction* action)
{
    TM_TRACE(TAG,"Action received");
    // request MDI to instanciate the tool (if any)
    createMdiView(action);
}

//
void                MainWindow::onRefreshToolbar()
{
    CoinMarketCap*  cmc         =Application::instance().cmc();
    /* Update clock         */
    QDateTime local=QDateTime::currentDateTime();
    QDateTime current=local;
    _clockLabel->setText(current.toString("HH:mm:ss"));
    /* Update BTC price     */
    QString   quoteAssetID=_quoteAssetCombo->currentData().toString();
    cmc->setQuoteAssetID(quoteAssetID);
    int index=0;
    for(int iWatch=0;iWatch<_watchAssets.length();iWatch++)
    {
        WatchAsset&     wa          =_watchAssets[iWatch];
        double          price       =1.0;
        if(cmc->assetPrice(quoteAssetID,wa.assetID,price,nullptr))
        {
            wa.action->setText(" "+cmc->formatFiatCurrency(price,quoteAssetID));
        }
        if(!wa.iconFound)
        {
            if(!cmc->assetIcon(wa.assetID).isNull())
            {
                wa.action->setIcon(cmc->assetIcon(wa.assetID));
                wa.iconFound=true;
            }
        }

        index++;
    }
    // calc total market cap
    {
        double totalMarketCap=cmc->totalMarketCap(quoteAssetID);
        _marketDataAssets->setText(""+cmc->formatFiatCurrency(totalMarketCap,quoteAssetID));
    }
}

//
void                MainWindow::onOpenMarketDataAssets()
{
    TM_TRACE(TAG,"Open market data assets");
    QAction     action;
    action.setProperty(PROP_TOOL_NAME,"marketdata/assets");
    createMdiView(&action);
}

//
void                MainWindow::onOpenWatchAsset()
{
    foreach(WatchAsset wa,_watchAssets)
    {
        if(wa.action==sender())
        {
            // open properties
            openAssetProperties(wa.assetID);
        }
    }
}

//
void                MainWindow::onOpenUserProfile()
{
    TM_TRACE(TAG,"Open user profile");
    UserProfileDialog  dlg(this);
    dlg.setModal(true);
    if(dlg.exec()==QDialog::Accepted)
    {

    }
}

//
void                MainWindow::onAccountChanged()
{
    TM_INFO(TAG,"Account changed");
    tmio::rest::Connection&   conn    =Application::instance().connection();
    _actionAccount->setText(conn.account()->displayName());
    if(conn.account()->avatarImage().length())
    {
        QImage image;
        image.loadFromData(conn.account()->avatarImage());
        QPixmap buffer = QPixmap::fromImage(image);
        QIcon icon;
        icon.addPixmap(buffer);
        _actionAccount->setIcon(icon);
    }
    else
    {
        TM_TRACE(TAG,"Avatar NOT found");
    }
}

//
void                MainWindow::onWatchAssetChanged(const QString& /*assetID*/,bool /*favorite*/)
{
    buildToolbarWatches();
    onRefreshToolbar();
}

