#include "ExchangeFeedsWidget.h"
#include "Application.h"
#include "ui_ToolBox.h"
#include <QToolButton>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QAction>

#include <Core.h>
#include <rest/Connection.h>
#include <rest/Request.h>
#include <md/Exchange.h>
#include "libs/ui/exchange/MarketWatchWidget.h"

//
#define TAG "ToolBox"

//
ExchangeFeedsWidget::ExchangeFeedsWidget(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::ToolBox)
{
    ui->setupUi(this);
    _layout = nullptr;
    //clear();

    md::MarketData*     marketData=Application::instance().marketData();
    foreach(QString id,marketData->exchanges())
    {
        md::Exchange*   exchange=marketData->exchange(id);
        if(exchange->feed())
        {
            Feed                feed;
            feed.exchange   =exchange->id();
            feed.name       =exchange->name();
            feed.widget     =new MarketWatchWidget(exchange->id());
            const int tabIndex=ui->tabWidget->addTab(feed.widget,Application::instance().cmc()->exchangeIcon(id),""/*,exchange->name()*/);
            ui->tabWidget->setTabToolTip(tabIndex,exchange->name());
            connect(feed.widget,SIGNAL(openTicker(QString,QString,QString)),this,SLOT(onOpenTicker(QString,QString,QString)));
            _feeds.append(feed);
        }
    }

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(onCurrentExchangeChanged(int)));

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->removeTab(0);
}

//
ExchangeFeedsWidget::~ExchangeFeedsWidget()
{
    delete ui;
}

//
void                ExchangeFeedsWidget::onOpenTicker(QString exchange,QString ticker,QString assetID)
{
    TM_TRACE(TAG,"Open ticker "+ticker+" on exchange "+exchange);
    QAction action("Open ticker");
    action.setProperty(PROP_TOOL_NAME,"exchange/openticker");
    //action.setProperty(PROP_TOOL_TOOL_DISPLAY_NAME,ticker);
    action.setProperty(PROP_TOOL_EXCHANGE_NAME,exchange);
    action.setProperty(PROP_TOOL_TICKER_NAME,ticker);
    action.setProperty(PROP_TOOL_ASSET_ID,assetID);
    emit sigActionTriggered(&action);
}

//
void                ExchangeFeedsWidget::onCurrentExchangeChanged(int index)
{
    TM_TRACE(TAG,QString("Current exchange changed: %1").arg(index));
    for(int i=0;i<ui->tabWidget->count();i++)
    {
        ui->tabWidget->setTabText(i, (i==index)?_feeds.at(i).name:"" );
        _feeds.at(i).widget->onRefreshTickers();
    }
}
