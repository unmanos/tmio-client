#include "Application.h"
#include "MainWindow.h"
#include "MarketWatchWidget.h"
#include "MarketWatchListWidget.h"
#include <QLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTimer>
#include <QLabel>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QApplication>

#include <Core.h>
#include <rest/Connection.h>
#include <rest/Request.h>
#include <md/Exchange.h>
#include <md/Asset.h>

#include "libs/ui/sort/GenericSortFilterProxyModel.h"

//
#define TAG "exchanges.MarketWatchWidget"
#define ITEMDATA_TIME           (Qt::UserRole+2)
#define ITEMDATA_TICKER         (Qt::UserRole+3)
#define ITEMDATA_ASSET_ID       (Qt::UserRole+4)
#define ITEMDATA_QUOTE_ID       (Qt::UserRole+5)

//
#define COL_TICKER              0
#define COL_PRICE               1
#define COL_CHANGE              2
//#define COL_VOLUME              3

//
MarketWatchWidget::MarketWatchWidget(const QString& exchangeID,QWidget *parent) : QWidget(parent)
{
    _exchange   =exchangeID;
    _marketData =Application::instance().marketData();
    _feed       =_marketData->exchangeFeed(exchangeID);
    connect(_feed,SIGNAL(tickerChanged(const tmio::md::ExchangeFeed::Ticker&)),this,SLOT(onTickerChanged(const tmio::md::ExchangeFeed::Ticker&)));
    QVBoxLayout*        mainLayout=new QVBoxLayout(this);
    setLayout(mainLayout);
    QFrame*             topFormFrame=new QFrame();
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    topFormFrame->setLayout(topFormLayout);
    mainLayout->addWidget(topFormFrame);

    _changePeriod=new QComboBox();
    _changePeriod->addItem("1m",    1*60*100);
    _changePeriod->addItem("5m",    5*60*100);
    _changePeriod->addItem("15m",   15*60*100);
    _changePeriod->addItem("1h",    60*60*100);
    _changePeriod->addItem("1d",    1440*60*100);
    _changePeriod->addItem("1w",    1440*7*60*100);
    _changePeriod->addItem("30d",   1440*30*60*100);
    _changePeriod->setCurrentIndex(4);
    connect(_changePeriod,SIGNAL(currentIndexChanged(int)),this,SLOT(onChangePeriodUpdate(int)));

    _volumePeriod=new QComboBox();
    _volumePeriod->addItem("5m",    5*60*100);
    _volumePeriod->addItem("15m",   15*60*100);
    _volumePeriod->addItem("1h",    60*60*100);
    _volumePeriod->addItem("1d",    24*60*60*100);
    _volumePeriod->addItem("1w",    7*24*60*60*100);
    _volumePeriod->addItem("30d",   30*24*60*60*100);
    _volumePeriod->setCurrentIndex(3);
    _volumePeriod->hide();
    connect(_volumePeriod,SIGNAL(currentIndexChanged(int)),this,SLOT(onChangePeriodUpdate(int)));

    //topFormLayout->addRow("Change period",_changePeriod);
    _filterTop100=new QCheckBox("Filter top 100");
    connect(_filterTop100,SIGNAL(toggled(bool)),this,SLOT(onChangeFilter()));

    topFormLayout->addWidget(_filterTop100);
    topFormLayout->addStretch(1);

    _columnType=new QComboBox();
    _columnType->addItem("Change period");
    //_columnType->addItem("Volume");
    connect(_columnType,SIGNAL(currentIndexChanged(int)),this,SLOT(onChangePeriodUpdate(int)));

    topFormLayout->addWidget(_columnType);
    topFormLayout->addWidget(_changePeriod);
    topFormLayout->addWidget(_volumePeriod);

    _currenciesTab=new QTabWidget();
    mainLayout->addWidget(_currenciesTab);

    md::Exchange*           exchange=_marketData->exchange(exchangeID);
    foreach(QString quoteSymbol,exchange->quoteSymbols())
    {
        const QString       quoteAssetID    =_marketData->loopupExchangeAsset(_exchange,quoteSymbol);
        if(quoteAssetID.length())
        {
            md::Asset*          quoteAsset      =_marketData->asset(quoteAssetID);
            _currencies.append(Currency(quoteAsset->exchange(exchangeID),quoteAsset->id(),quoteAsset->name(),8));
        }
        else
        {
            _currencies.append(Currency(quoteSymbol,QString(),QString(),8));
        }
    }

    connect(_currenciesTab,SIGNAL(currentChanged(int)),this,SLOT(onCurrentQuoteChanged(int)));
    for(int i=0;i<_currencies.length();i++)
    {
        Currency currency=_currencies.at(i);
        currency.pairs=new QTreeView();
        currency.pairs->setRootIsDecorated(false);
        currency.sortFilter=new GenericSortFilterProxyModel(currency.pairs);

        currency.pairs->setStyleSheet(
            "QTreeView::indicator:unchecked {image: url(:/icons/common/checkbox-star-unchecked.png);}"
            "QTreeView::indicator:checked {image: url(:/icons/common/checkbox-star-checked.png);}"
          );

        //currency.pairs->setModel(sortFilter);
        //currency.pairs->set

        currency.pairs->setSortingEnabled(true);
        currency.pairs->setAlternatingRowColors(true);
        currency.pairs->setEditTriggers(QAbstractItemView::NoEditTriggers);

        if(currency.assetID.length())
        {
            const int tabIndex=_currenciesTab->addTab(currency.pairs,Application::instance().cmc()->assetIcon(currency.assetID),"");
            _currenciesTab->setTabToolTip(tabIndex,currency.assetName);
        }

        QStandardItemModel* model=new QStandardItemModel();
        //QStandardItemModel* model=new GenericSortFilterProxyModel();
        currency.model=model;
        currency.sortFilter->setSourceModel(model);
        currency.pairs->setModel(currency.sortFilter);

        model->setHorizontalHeaderItem(COL_TICKER,  new QStandardItem(tr("Ticker")));
        model->setHorizontalHeaderItem(COL_PRICE,   new QStandardItem(tr("Price")));
        model->setHorizontalHeaderItem(COL_CHANGE,  new QStandardItem(tr("Change")));

        QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
        //listHeader->stretchLastSection();
        listHeader->setSortIndicatorShown(true);
        //listHeader->setMinimumWidth();
        currency.pairs->setHeader(listHeader);
        listHeader->setSortIndicator(2,Qt::SortOrder::DescendingOrder);
        //listHeader->setDefaultSectionSize(10);
        listHeader->setSectionsMovable(true);
        listHeader->resizeSection(COL_TICKER,127);
        listHeader->resizeSection(COL_PRICE,110);
        listHeader->resizeSection(COL_CHANGE,85);
        //connect()
        connect(currency.pairs,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onOpenTicker(QModelIndex)));
        connect(currency.model,SIGNAL(itemChanged(QStandardItem*)),SLOT(onItemChanged(QStandardItem*)));

        currency.pairs->setSelectionMode(QAbstractItemView::SingleSelection);
        currency.pairs->setUniformRowHeights(true);
        connect(currency.pairs->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

        connect(currency.pairs,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

        _currencies[i]=currency;
    }
    // setup periodic updates
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onRefreshTickers()));
    timer->start(500);
    //
    connect(_marketData,&md::MarketData::favoriteTickerChanged,this,&MarketWatchWidget::onFavoriteTickerChanged);

    // first update
    updateData(false);
}

//
void        MarketWatchWidget::onRefreshTickers()
{
    if(isVisible())
        updateData(false);
}

//
void        MarketWatchWidget::onCurrentRowChanged(QModelIndex index,QModelIndex /*oldindex*/)
{
    QModelIndex             firstIndex  =index.sibling(index.row(),0);
    QString                 ticker      =firstIndex.data().toString();
    QString                 assetID     =firstIndex.data(ITEMDATA_ASSET_ID).toString();
    if(assetID.length())
    {
        TM_TRACE(TAG,QString("Activate ticker index %1 (%2)").arg(index.row()).arg(ticker));
        Application::instance().mainWindow()->openAssetProperties(assetID);
    }
    else
    {
        TM_WARNING(TAG,QString("No asset ID for: %1").arg(ticker));
    }
}

//
void        MarketWatchWidget::onOpenTicker(QModelIndex index)
{
    QModelIndex             firstIndex  =index.sibling(index.row(),0);
    QString                 ticker      =firstIndex.data(ITEMDATA_TICKER).toString();
    QString                 assetID     =firstIndex.data(ITEMDATA_ASSET_ID).toString();
    TM_TRACE(TAG,QString("Open ticker index %2 (%3)").arg(index.row()).arg(ticker));
    emit openTicker(_exchange,ticker,assetID);
}

//
void         MarketWatchWidget::onChangePeriodUpdate(int)
{
    _changePeriod->setVisible(_columnType->currentIndex()==0);
    _volumePeriod->setVisible(_columnType->currentIndex()==1);
    int         volumePeriod=_volumePeriod->currentData().toInt();
    QVector<tmio::md::ExchangeFeed::Ticker> tickers=_feed->tickers();
    for(int i=0;i<_currencies.length();i++)
    {
        Currency& currency=_currencies[i];//.at(i);
        if(_columnType->currentIndex()==0)
            currency.model->setHorizontalHeaderItem(COL_CHANGE, new QStandardItem(tr("Change")));
        if(_columnType->currentIndex()==1)
            currency.model->setHorizontalHeaderItem(COL_CHANGE, new QStandardItem(tr("Volume")));
        bool        firstVolume=true;
        double      minVolume=0,maxVolume=0;
        currency.volumeDivider=1;
        for(int i=0;i<tickers.count();i++)
        {
            const tmio::md::ExchangeFeed::Ticker& ticker=tickers[i];
            if(ticker.name.endsWith(currency.symbol))
            {
                if(ticker.quoteVolumes.contains(volumePeriod))
                {
                    double v=ticker.quoteVolumes[volumePeriod];
                    minVolume=firstVolume?v:qMin(minVolume,v);
                    maxVolume=firstVolume?v:qMax(maxVolume,v);
                    firstVolume=false;
                }
            }
        }
        if(minVolume>=10000)
            currency.volumeDivider=1000;
        if(minVolume>=10000000)
            currency.volumeDivider=1000000;
        if(minVolume>=10000000000)
            currency.volumeDivider=1000000000;
        if(maxVolume<=10000)
            currency.volumeDivider=-1;
        if(maxVolume<=1000)
            currency.volumeDivider=-2;
        if(maxVolume<=100)
            currency.volumeDivider=-3;
        if(maxVolume<=10)
            currency.volumeDivider=-4;
    }
    updateData(true);
}

//
void         MarketWatchWidget::onChangeFilter()
{
    for(int i=0;i<_currencies.length();i++)
    {
        Currency currency=_currencies.at(i);
        if(currency.model->hasChildren())
        {
            currency.model->removeRows(0, currency.model->rowCount());
        }
    }
    updateData(false);
}

//
void         MarketWatchWidget::onTickerChanged(const tmio::md::ExchangeFeed::Ticker& ticker)
{
    _externalUpdate     =true;
    updateTicker(ticker);
    _externalUpdate     =false;
}

//
void         MarketWatchWidget::onItemChanged(QStandardItem* item)
{
    const QString       tickerName  =item->data(ITEMDATA_TICKER).toString();
    if( (!tickerName.isEmpty())&&(!_externalUpdate) )
    {
        _marketData->setFavoriteTicker(_exchange,tickerName,(item->checkState()==Qt::Checked));
    }
}

//
void        MarketWatchWidget::onFavoriteTickerChanged(const QString& exchange,const QString& /*ticker*/,bool /*favorite*/)
{
    if(exchange==_exchange)
        updateData(false);
}

//
void         MarketWatchWidget::updateData(bool /*resetChange*/)
{
    QVector<tmio::md::ExchangeFeed::Ticker> tickers=_feed->tickers();
    for(int i=0;i<tickers.count();i++)
    {
        onTickerChanged(tickers[i]);
    }
}

void         MarketWatchWidget::updateTicker(const tmio::md::ExchangeFeed::Ticker& ticker)
{
    if(!isVisible())
        return;
    //Application::instance().
    QPalette            palette =qApp->palette();
    const Style&        style   =Application::instance().style();
    CoinMarketCap*      cmc=Application::instance().cmc();
    md::MarketData*     md=Application::instance().marketData();
    const QString       name=ticker.name;//ticker["ticker"].toString();
    //QString             open=QString::number(ticker["open"].toDouble(),'f',8);
    QString             price=QString::number(ticker.price,'f',8);
    const QString       assetID=ticker.assetID;
    const QString       quoteID=ticker.quoteID;
    QDateTime           time=ticker.time;//QDateTime::fromString(ticker.value("time").toString(),Qt::ISODate);
    QDateTime           previousTime=QDateTime::fromSecsSinceEpoch(0);
    bool                top100=_filterTop100->checkState()==Qt::Checked;
    md::Asset*          asset=md->asset(assetID);
    int                 assetRank=cmc->assetRank(assetID);
    if( (top100)&&( (assetRank>100)||(assetRank==0) ) )
    {
        return;
    }
    for(int i=0;i<_currencies.length();i++)
    {
        const Currency& currency=_currencies.at(i);
        if(name.endsWith("-"+currency.symbol))
        {
            if(!currency.pairs->isVisible())
                return;
            QString     tickerText  =QString(name).replace("-"," / ");
            QList<QStandardItem*> l=currency.model->findItems(tickerText,Qt::MatchFlag::MatchExactly,0);
            QStandardItem* item=l.length()?l.at(0):nullptr;
            if(!item)
            {
                item=new QStandardItem();
                item->setCheckable(true);
                l.append(item);
                l.append(new QStandardItem());
                l.append(new QStandardItem());
                l[1]->setTextAlignment(Qt::AlignRight);
                l[2]->setTextAlignment(Qt::AlignRight);

                currency.model->appendRow(l);
                item->setText(tickerText);
                item->setData(time,ITEMDATA_TIME);
                item->setData(name,ITEMDATA_TICKER);
                item->setData(assetID,ITEMDATA_ASSET_ID);
                item->setData(quoteID,ITEMDATA_QUOTE_ID);
            }
            else
            {
                previousTime=item->data(ITEMDATA_TIME).toDateTime();
            }
            if(assetID.length())
            {
                if(asset)
                {
                    QString toolTip=QString("<b>%1</b><br>Rank: %2").arg(asset->name()).arg(assetRank?QString::number(assetRank):"?");
                    item->setToolTip(toolTip);
                }
                //sortFilter->index()
                if(item->icon().isNull())
                {
                    const QRect visualRect=currency.pairs->visualRect(currency.sortFilter->mapFromSource(item->index()));
                    const bool visible=(visualRect.top()>=0)&&(visualRect.bottom()<=currency.pairs->height());
                    if( (visible)&&(item->icon().isNull()) )
                    {
                        item->setIcon(Application::instance().cmc()->assetIcon(assetID));
                    }
                }
            }
            else
            {
            }
            item->setCheckState(md->favoriteTicker(_exchange,name)?Qt::Checked:Qt::Unchecked);

            QStandardItem*  iPrice  =currency.model->item(item->row(),COL_PRICE);
            QStandardItem*  iChange =currency.model->item(item->row(),COL_CHANGE);
            double   p0=iPrice->text().toDouble();
            double   p1=price.toDouble();
            if(time!=previousTime)
            {
                if(previousTime.toSecsSinceEpoch()==0)
                    p0=p1;
                iPrice->setText(cmc->formatAmount(price.toDouble(),currency.precision));
                iPrice->setData(QString(price).toDouble(),USER_ROLE_SORT_NUMERIC);
                //iPrice->setData();
                if(p1>p0)
                {
                    // green
                    iPrice->setData(style.color(Style::colorTextChangeIncrease),Qt::ForegroundRole);
                }
                else if(p1<p0)
                {
                    // red
                    iPrice->setData(style.color(Style::colorTextChangeDecrease),Qt::ForegroundRole);
                }
                else
                {
                    // same
                    iPrice->setData(palette.color(QPalette::Text),Qt::ForegroundRole);
                }
                item->setData(time,ITEMDATA_TIME);
            }
            // calc percent change
            iChange->setText("");
            iChange->setData((int)0,USER_ROLE_SORT_NUMERIC);
            switch(_columnType->currentIndex())
            {
                //
                case 0:
                {
                    int         changePeriod=_changePeriod->currentData().toInt();
                    if(ticker.previousPrices.contains(changePeriod))
                    {
                        double oldp=ticker.previousPrices[changePeriod];
                        double percentChange=((ticker.price-oldp)/qAbs(oldp))*100;
                        iChange->setText(QString::number(qAbs(percentChange),'f',2)+"%");
                        iChange->setData((percentChange<0)?style.color(Style::colorTextChangeDecrease):style.color(Style::colorTextChangeIncrease),Qt::ForegroundRole);
                        iChange->setData(percentChange,USER_ROLE_SORT_NUMERIC);
                    }
                }
                break;
                //
                case 1:
                {
                    // volume
                    int         volumePeriod=_volumePeriod->currentData().toInt();
                    if(ticker.quoteVolumes.contains(volumePeriod))
                    {
                        double  v=ticker.quoteVolumes[volumePeriod];
                        QString     suffix="";
                        int         precision=0;
                        double      divider=1;
                        if(currency.volumeDivider>0)
                        {
                            divider=(double)currency.volumeDivider;
                            if(currency.volumeDivider==1000) suffix="K";
                            if(currency.volumeDivider==1000000) suffix="M";
                            if(currency.volumeDivider==1000000000) suffix="B";
                        }
                        else
                        {
                            precision=-currency.volumeDivider;
                        }
                        iChange->setText(Application::instance().cmc()->formatAmount(v/divider,precision)+suffix);
                        iChange->setData(v,USER_ROLE_SORT_NUMERIC);
                        iChange->setData(palette.color(QPalette::Text),Qt::ForegroundRole);
                    }
                }
                break;
                //
            }
        }
    }
}

//
void        MarketWatchWidget::onCurrentQuoteChanged(int index)
{
    TM_TRACE(TAG,QString("Current quote changed: %1").arg(index));
    for(int i=0;i<_currenciesTab->count();i++)
    {
        if(!_currenciesTab->tabIcon(i).isNull())
            _currenciesTab->setTabText(i, (i==index)?_currencies.at(i).symbol:"");
    }
    updateData(false);
}
