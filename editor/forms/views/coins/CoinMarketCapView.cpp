#include "Application.h"
#include "MainWindow.h"
#include "CoinMarketCapView.h"
#include "libs/ui/sort/GenericSortFilterProxyModel.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTimer>

#define TAG "views.CoinMarketCapView"
#define ITEMDATA_CMC_ID         (Qt::UserRole+4)
#define ITEMDATA_CMC_LINK       (Qt::UserRole+5)

//
#define COL_RANK                0
#define COL_NAME                1
#define COL_SYMBOL              2
#define COL_PRICE               3
#define COL_MARKET_CAP          4
#define COL_CIRCULATING_SUPPLY  5
//#define COL_CHANGE_1H           6
//#define COL_CHANGE_1D           7
//#define COL_CHANGE_1W           8
#define COL_COUNT               6


//
CoinMarketCapView::CoinMarketCapView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout*        mainLayout=new QVBoxLayout();
    setLayout(mainLayout);

    //_lookup.reserve(4096);

    _treeView           =new QTreeView(this);
    _treeView->setSortingEnabled(true);
    _treeView->setAlternatingRowColors(true);
    _treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    _sortFilter=new GenericSortFilterProxyModel(_treeView);


    _model              =new QStandardItemModel();
    _model->setHorizontalHeaderItem(COL_RANK, new QStandardItem(tr("Rank")));
    _model->setHorizontalHeaderItem(COL_NAME, new QStandardItem(tr("Name")));
    _model->setHorizontalHeaderItem(COL_SYMBOL, new QStandardItem(tr("Symbol")));
    _model->setHorizontalHeaderItem(COL_PRICE, new QStandardItem(tr("Price")));
    _model->setHorizontalHeaderItem(COL_MARKET_CAP, new QStandardItem(tr("Market Cap")));
    _model->setHorizontalHeaderItem(COL_CIRCULATING_SUPPLY, new QStandardItem(tr("Circulating supply")));
    #if(COL_CHANGE_1H)
    _model->setHorizontalHeaderItem(COL_CHANGE_1H, new QStandardItem(tr("% hour")));
    _model->setHorizontalHeaderItem(COL_CHANGE_1D, new QStandardItem(tr("% day")));
    _model->setHorizontalHeaderItem(COL_CHANGE_1W, new QStandardItem(tr("% week")));
    #endif

    //_treeView->setModel(_model);
    //sortFilter->setDynamicSortFilter(true);
    _sortFilter->setSourceModel(_model);
    _treeView->setModel(_sortFilter);


    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(true);
    _treeView->setHeader(listHeader);
    listHeader->setSortIndicator(0,Qt::SortOrder::AscendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->resizeSection(COL_RANK,60);
    listHeader->resizeSection(COL_NAME,200);
    listHeader->resizeSection(COL_SYMBOL,110);
    listHeader->resizeSection(COL_PRICE,110);
    listHeader->resizeSection(COL_MARKET_CAP,190);
    listHeader->resizeSection(COL_CIRCULATING_SUPPLY,190);
    #if(COL_CHANGE_1H)
    listHeader->resizeSection(COL_CHANGE_1H,100);
    listHeader->resizeSection(COL_CHANGE_1D,100);
    listHeader->resizeSection(COL_CHANGE_1W,100);
    #endif

    _treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    _treeView->setUniformRowHeights(true);

    connect(_treeView->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

    mainLayout->addWidget(_treeView);
    mainLayout->addWidget(new QLabel("Data kindly provided by coinmarketcap.com"));

    updateData();

    QTimer*     timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(onUpdateData()));
    timer->start(1000);

    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));
}

//
void            CoinMarketCapView::updateData()
{
}

//
void            CoinMarketCapView::onUpdateData()
{
    if(_treeView->isVisible())
    {
        updateData();
    }
}

//
void            CoinMarketCapView::onQuoteAssetChanged()
{
    updateData();
}

//
void            CoinMarketCapView::onCurrentRowChanged(QModelIndex index,QModelIndex /*oldIndex*/)
{
    QModelIndex             firstIndex=index.sibling(index.row(),0);
    QString                 ticker=firstIndex.data().toString();
    int                     cmcId=firstIndex.data(ITEMDATA_CMC_ID).toInt();
    if(cmcId)
    {
        TM_TRACE(TAG,QString("Activate ticker index %2 (%3)").arg(index.row()).arg(ticker));
        TM_ERROR(TAG,"DEPRECATED");
    }
}
