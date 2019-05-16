#include <QFrame>
#include <QBoxLayout>
#include <QIcon>
#include <QHeaderView>

#include "Application.h"
#include "MainWindow.h"
#include "ExchangeLiveView.h"

//
#define COL_TICKER          0
#define COL_NAME            1
#define COL_SYMBOL          2
#define COL_ASSET           3
#define COL_VCHANGE_1M_1D   4
#define COL_VCHANGE_5M_1D   5
#define COL_VCHANGE_15M_1D  6
#define COL_VCHANGE_1H_7D   7
#define COL_COUNT           8

//
#define TAG "ExchangeLiveView"
#define ITEMDATA_TICKER         (Qt::UserRole+3)
#define ITEMDATA_CMC_ID         (Qt::UserRole+4)

//
ExchangeLiveView::ExchangeLiveView(const QString& exchange,QWidget *parent) : QWidget(parent)
{
    // feed
    _exchange   =exchange;
    _marketData =Application::instance().marketData();
    _feed       =_marketData->exchangeFeed(exchange);
    connect(_feed,SIGNAL(tickerChanged(const tmio::md::ExchangeFeed::Ticker&)),this,SLOT(onTickerChanged(const tmio::md::ExchangeFeed::Ticker&)));
    // layout
    setLayout(new QVBoxLayout());
    setWindowTitle(tr("Live view"));
    // signals
    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));
    // title frame
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    QFrame*             titleFrame=new QFrame();
    titleFrame->setLayout(topFormLayout);
    topFormLayout->setMargin(0);
    topFormLayout->addSpacing(8);
    //titleFrame->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    layout()->addWidget(titleFrame);

    // tree
    _treeView           =new QTreeView(this);
    _treeView->setSortingEnabled(true);
    _treeView->setAlternatingRowColors(true);
    _treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    _sortFilter=new GenericSortFilterProxyModel(_treeView);


    _model              =new QStandardItemModel();
    //_model->setHorizontalHeaderItem(COL_RANK, new QStandardItem(tr("Rank")));
    _model->setHorizontalHeaderItem(COL_TICKER, new QStandardItem(tr("Ticker")));
    _model->setHorizontalHeaderItem(COL_NAME, new QStandardItem(tr("Name")));
    _model->setHorizontalHeaderItem(COL_SYMBOL, new QStandardItem(tr("Symbol")));
    _model->setHorizontalHeaderItem(COL_ASSET, new QStandardItem(tr("Asset")));
    _model->setHorizontalHeaderItem(COL_VCHANGE_1M_1D, new QStandardItem(tr("VC 1m/1d")));
    _model->setHorizontalHeaderItem(COL_VCHANGE_5M_1D, new QStandardItem(tr("VC 5m/1d")));
    _model->setHorizontalHeaderItem(COL_VCHANGE_15M_1D, new QStandardItem(tr("VC 15m/1d")));
    _model->setHorizontalHeaderItem(COL_VCHANGE_1H_7D, new QStandardItem(tr("VC 1h/7d")));

    /*_model->setHorizontalHeaderItem(COL_EXCHANGE, new QStandardItem(tr("Exchange")));
    _model->setHorizontalHeaderItem(COL_NAME, new QStandardItem(tr("Name")));
    _model->setHorizontalHeaderItem(COL_SYMBOL, new QStandardItem(tr("Symbol")));
    _model->setHorizontalHeaderItem(COL_UNIT_PRICE, new QStandardItem(tr("Unit price")));
    _model->setHorizontalHeaderItem(COL_AMOUNT, new QStandardItem(tr("Amount")));
    _model->setHorizontalHeaderItem(COL_TOTAL, new QStandardItem(tr("Total")));*/
    _sortFilter->setSourceModel(_model);
    _treeView->setModel(_sortFilter);
    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(false);
    _treeView->setHeader(listHeader);
    //listHeader->setSortIndicator(0,Qt::SortOrder::AscendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->hideSection(COL_TICKER);
    listHeader->resizeSection(COL_TICKER,   100);
    listHeader->resizeSection(COL_NAME,     200);
    listHeader->resizeSection(COL_SYMBOL,   80);
    listHeader->resizeSection(COL_ASSET,    80);
    listHeader->resizeSection(COL_VCHANGE_1M_1D,    80);
    listHeader->resizeSection(COL_VCHANGE_5M_1D,    80);
    listHeader->resizeSection(COL_VCHANGE_15M_1D,   80);
    listHeader->resizeSection(COL_VCHANGE_1H_7D,    80);
    _treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    _treeView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    connect(_treeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onOpenTicker(QModelIndex)));
    connect(_treeView->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

    layout()->addWidget(_treeView);
    // first update
    updateData(true);
}

//
void                    ExchangeLiveView::updateData(bool /*resetChange*/)
{
    QVector<tmio::md::ExchangeFeed::Ticker> tickers=_feed->tickers();
    for(int i=0;i<tickers.count();i++)
    {
        onTickerChanged(tickers[i]);
    }
}

//
void                    ExchangeLiveView::onUpdate()
{
}

//
void                    ExchangeLiveView::onQuoteAssetChanged()
{
}

//
void                    ExchangeLiveView::onCurrentRowChanged(QModelIndex index,QModelIndex /*oldIndex*/)
{
    QModelIndex             firstIndex=index.sibling(index.row(),0);
    QString                 ticker=firstIndex.data().toString();
    int                     cmcId=firstIndex.data(ITEMDATA_CMC_ID).toInt();
    if(cmcId)
    {
        TM_TRACE(TAG,QString("Activate ticker index %1 (%2)").arg(index.row()).arg(ticker));
        if(cmcId>=0)
        {
            //Application::instance().mainWindow()->openCoinProperties(cmcId);
            TM_TRACE(TAG,"ExchangeLiveView::onCurrentRowChanged");
        }
        else
        {
            TM_WARNING(TAG,QString("No CMC coin ID: %1").arg(ticker));
        }
    }
}

//
void                    ExchangeLiveView::onOpenTicker(QModelIndex index)
{
    QModelIndex             firstIndex=index.sibling(index.row(),0);
    QString                 ticker=firstIndex.data(ITEMDATA_TICKER).toString();
    TM_TRACE(TAG,QString("Open ticker index %2 (%3)").arg(index.row()).arg(ticker));
    //emit openTicker(tmio::md::marketName(_exchange),ticker,cmcId);
}

//
void                    ExchangeLiveView::onTickerChanged(const tmio::md::ExchangeFeed::Ticker& /*ticker*/)
{
}
