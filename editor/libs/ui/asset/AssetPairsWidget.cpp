#include "AssetPairsWidget.h"
#include <QTreeView>
#include <QHeaderView>
#include <QModelIndex>
#include <QGridLayout>

#include <md/Asset.h>
#include <md/Exchange.h>

#include "Application.h"
#include "libs/ui/sort/GenericSortFilterProxyModel.h"

#define TAG                     "AssetPairsWidget"

#define PAIRS_COL_EXCHANGE      0
#define PAIRS_COL_NAME          1
#define PAIRS_COL_PRICE         2
#define PAIRS_COL_COUNT         3

#define PAIRS_ITEMDATA_EXCHANGE (Qt::UserRole+2)
#define PAIRS_ITEMDATA_TICKER   (Qt::UserRole+3)

using namespace tmio;

//
AssetPairsWidget::AssetPairsWidget(const QString& assetID,QWidget *parent) : QWidget(parent), _assetID(assetID)
{
    md::MarketData*     md      =Application::instance().marketData();
    md::Asset*          asset   =md->asset(_assetID);

    setLayout(new QGridLayout());
    QTreeView*  treeView           =new QTreeView();
    layout()->addWidget(treeView);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    treeView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    treeView->setRootIsDecorated(false);
    treeView->setSortingEnabled(true);
    treeView->setAlternatingRowColors(true);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setStyleSheet(
        "QTreeView::indicator:unchecked {image: url(:/icons/common/checkbox-star-unchecked.png);}"
        "QTreeView::indicator:checked {image: url(:/icons/common/checkbox-star-checked.png);}"
      );
    GenericSortFilterProxyModel*    sortFilter          =new GenericSortFilterProxyModel(treeView);
    _pairsModel=new QStandardItemModel();
    _pairsModel->setHorizontalHeaderItem(PAIRS_COL_EXCHANGE, new QStandardItem(tr("Exchange")));
    _pairsModel->setHorizontalHeaderItem(PAIRS_COL_NAME, new QStandardItem(tr("Pair")));
    _pairsModel->setHorizontalHeaderItem(PAIRS_COL_PRICE, new QStandardItem(tr("Price")));
    sortFilter->setSourceModel(_pairsModel);
    treeView->setModel(sortFilter);
    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(true);
    treeView->setHeader(listHeader);
    listHeader->setSortIndicator(PAIRS_COL_EXCHANGE,Qt::SortOrder::AscendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->resizeSection(PAIRS_COL_EXCHANGE,105);
    listHeader->resizeSection(PAIRS_COL_NAME,125);
    listHeader->resizeSection(PAIRS_COL_PRICE,90);

    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setUniformRowHeights(true);

    //connect(treeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onOpenTicker(QModelIndex)));

    connect(asset,&md::Asset::priceChanged,this,&AssetPairsWidget::onPriceChanged);
    connect(md,&md::MarketData::favoriteTickerChanged,this,&AssetPairsWidget::onFavoriteTickerChanged);
    connect(_pairsModel,SIGNAL(itemChanged(QStandardItem*)),SLOT(onPairItemChanged(QStandardItem*)));
    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));
    connect(treeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onOpenPair(QModelIndex)));

    refreshPairs();
}

//
void                            AssetPairsWidget::refreshPairs()
{
    CoinMarketCap*                    cmc               =Application::instance().cmc();
    md::MarketData*                   marketData        =Application::instance().marketData();
    md::Asset*                        asset             =marketData->asset(_assetID);
    foreach(const md::Asset::Quote& quote, asset->quotes())
    {
        if(quote.reverse)
            continue;
        //md::
        const QString           quoteID     =quote.quoteID;
        md::Asset*              quoteAsset  =marketData->asset(quoteID);
        md::Exchange*           exchange    =marketData->exchange(quote.exchange);
        const QString           key         =exchange->id()+"."+quoteID;
        QStandardItem*          item        =_pairsLookup[key];
        QVector<QStandardItem*> row;
        QString                 left        =asset->exchange(exchange->id());
        QString                 right       =quoteAsset->exchange(exchange->id());
        if(!item)
        {
            for(int i=0;i<PAIRS_COL_COUNT;i++)
            {
                QStandardItem*   rowItem=new QStandardItem("?");
                row.append(rowItem);
                if(i==0)
                {
                    item=rowItem;
                    rowItem->setCheckable(true);
                }
            }
            row[PAIRS_COL_EXCHANGE]->setIcon(cmc->exchangeIcon(exchange->id()));
            row[PAIRS_COL_EXCHANGE]->setText(exchange->name());
            row[PAIRS_COL_NAME]->setText(left+" / "+right);
            row[PAIRS_COL_NAME]->setIcon(cmc->assetIcon(quoteID));
            item->setData(exchange->id(),PAIRS_ITEMDATA_EXCHANGE);
            item->setData(left+"-"+right,PAIRS_ITEMDATA_TICKER);
            _pairsModel->appendRow(row.toList());
            _pairsLookup[key]=item;
        }
        else
        {
            for(int i=0;i<PAIRS_COL_COUNT;i++)
            {
                row.append((i==0)?item:_pairsModel->item(item->row(),i));
            }
        }
        // calculate price
        double price=quote.price;
        if(cmc->assetPrice(cmc->quoteAssetID(),quoteAsset->id(),price,nullptr))
        {
            row[PAIRS_COL_PRICE]->setText(cmc->formatFiatCurrency(price,cmc->quoteAssetID()));
            row[PAIRS_COL_PRICE]->setData(price,USER_ROLE_SORT_NUMERIC);
        }
        else
        {
            row[PAIRS_COL_PRICE]->setText("?");
            row[PAIRS_COL_PRICE]->setData(static_cast<double>(0),USER_ROLE_SORT_NUMERIC);
        }
        row[0]->setCheckState(marketData->favoriteTicker(quote.exchange,left+"-"+right)?Qt::Checked:Qt::Unchecked);
    }
}

//
void                            AssetPairsWidget::onPriceChanged()
{
    refreshPairs();
}

//
void                            AssetPairsWidget::onQuoteAssetChanged()
{
    refreshPairs();
}

//
void                            AssetPairsWidget::onFavoriteTickerChanged(const QString& /*exchange*/,const QString& /*ticker*/,bool /*favorite*/)
{
    refreshPairs();
}

//
void                            AssetPairsWidget::onPairItemChanged(QStandardItem* item)
{
    md::MarketData*         md          =Application::instance().marketData();
    QString                 ticker      =item->data(PAIRS_ITEMDATA_TICKER).toString();
    QString                 exchange    =item->data(PAIRS_ITEMDATA_EXCHANGE).toString();
    if(!ticker.isEmpty())
    {
        md->setFavoriteTicker(exchange,ticker,(item->checkState()==Qt::Checked));
    }
}

//
void                            AssetPairsWidget::onOpenPair(QModelIndex index)
{
    QModelIndex             firstIndex  =index.sibling(index.row(),0);
    QString                 ticker      =firstIndex.data(PAIRS_ITEMDATA_TICKER).toString();
    QString                 exchange    =firstIndex.data(PAIRS_ITEMDATA_EXCHANGE).toString();
    emit openPair(exchange,ticker);
}

