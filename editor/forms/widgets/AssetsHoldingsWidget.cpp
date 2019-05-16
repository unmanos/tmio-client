#include "Application.h"
#include "MainWindow.h"
#include "AssetsHoldingsWidget.h"
#include "config.h"
#include "libs/ui/sort/GenericSortFilterProxyModel.h"

#include <md/Asset.h>
#include <md/Exchange.h>
#include <am/Wallet.h>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QFrame>

//
using namespace tmio;

//
#define TAG                     "AssetsHoldingsWidget"

#define COL_ID                  0
#define COL_NAME                1
#define COL_SYMBOL              2
#define COL_PRICE               3
#define COL_AMOUNT              4
#define COL_TOTAL               5
#define COL_COUNT               6

#define ITEMDATA_ASSET_ID       (Qt::UserRole+1)
#define ITEMDATA_EXCHANGE       (Qt::UserRole+2)
#define ITEMDATA_TICKER         (Qt::UserRole+3)

//
QString                         _privateData(const QString& text)
{
    #if TMEDITOR_HIDE_WEALTH
    return(QString("[hidden]"));
    #else
    return(text);
    #endif
}

//
AssetsHoldingsWidget::AssetsHoldingsWidget(QWidget *parent) : QWidget(parent)
{
    setLayout(new QVBoxLayout());

    // title frame
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    QFrame*             titleFrame=new QFrame();
    titleFrame->setLayout(topFormLayout);
    topFormLayout->setMargin(0);
    topFormLayout->addSpacing(8);
    // add coin icon
    {
        //QLabel*             coinIcon=new QLabel();
        //coinIcon->setPixmap(QIcon(":/icons/assets.png").pixmap(30,30));
        //topFormLayout->addWidget(coinIcon);
        //topFormLayout->addStretch(1);
    }
    // add confirmed balance
    {
        _labelTotalBalance = new QLabel();
        _labelTotalBalanceQuote = new QLabel();
        topFormLayout->addWidget(_labelTotalBalance);
        topFormLayout->addSpacing(20);
        topFormLayout->addWidget(_labelTotalBalanceQuote);
        topFormLayout->addStretch(1);
    }

    QTreeView*  treeView           =new QTreeView();
    //treeView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    treeView->setRootIsDecorated(false);
    treeView->setSortingEnabled(true);
    treeView->setAlternatingRowColors(true);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setStyleSheet(
        "QTreeView::indicator:unchecked {image: url(:/icons/common/checkbox-star-unchecked.png);}"
        "QTreeView::indicator:checked {image: url(:/icons/common/checkbox-star-checked.png);}"
      );
    GenericSortFilterProxyModel*    sortFilter          =new GenericSortFilterProxyModel(treeView);
    _model=new QStandardItemModel();
    _model->setHorizontalHeaderItem(COL_ID, new QStandardItem(tr("ID")));
    _model->setHorizontalHeaderItem(COL_NAME, new QStandardItem(tr("Name")));
    _model->setHorizontalHeaderItem(COL_SYMBOL, new QStandardItem(tr("Symbol")));
    _model->setHorizontalHeaderItem(COL_PRICE, new QStandardItem(tr("Price")));
    _model->setHorizontalHeaderItem(COL_AMOUNT, new QStandardItem(tr("Amount")));
    _model->setHorizontalHeaderItem(COL_TOTAL, new QStandardItem(tr("Total")));

    sortFilter->setSourceModel(_model);
    treeView->setModel(sortFilter);

    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(true);
    treeView->setHeader(listHeader);
    listHeader->setSortIndicator(COL_TOTAL,Qt::SortOrder::DescendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->hideSection(COL_ID);
    listHeader->resizeSection(COL_NAME,170);
    listHeader->resizeSection(COL_SYMBOL,80);
    listHeader->resizeSection(COL_PRICE,100);
    listHeader->resizeSection(COL_AMOUNT,100);
    listHeader->resizeSection(COL_TOTAL,100);

    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setUniformRowHeights(true);

    tmio::am::AssetsManager*    am      =Application::instance().assetsManager();
    connect(am,&am::AssetsManager::walletsChanged,this,&AssetsHoldingsWidget::onUpdate);

    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onUpdate()));

    connect(treeView->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,SIGNAL(timeout()),this,SLOT(onUpdate()));
    updateTimer->start(1000/*10s update*/);
    //
    refreshHoldings();

    layout()->addWidget(treeView);
    layout()->setMargin(0);
    layout()->setSpacing(0);
    layout()->addWidget(titleFrame);

    titleFrame->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    resize(400,height());

}

//
void                        AssetsHoldingsWidget::refreshHoldings()
{
    md::MarketData*             md              =Application::instance().marketData();
    tmio::am::AssetsManager*    am              =Application::instance().assetsManager();
    CoinMarketCap*              cmc             =Application::instance().cmc();
    double                      btcAmount       =0;
    QMap<QString,Row>           untouchedRows   =_rows;
    foreach(am::Wallet::Asset h,am->assets())
    {
        md::Asset*            asset =md->asset(h.assetID);
        const QString         key   =h.assetID.length()?("id:"+h.assetID):("symbol:"+h.symbol);
        QList<QStandardItem*> row   =_rows.contains(key)?_rows[key]:Row();
        QStandardItem*        item  =row.length()?row.at(0):nullptr;
        if(!item)
        {
            item=new QStandardItem();
            row.append(item);
            for(int i=1;i<COL_COUNT;i++)
            {
                row.append(new QStandardItem());
            }
            item->setText(key);
            row[COL_NAME]->setText(asset?asset->name():h.symbol);
            row[COL_SYMBOL]->setText(h.symbol);
            row[COL_PRICE]->setTextAlignment(Qt::AlignRight);
            row[COL_AMOUNT]->setTextAlignment(Qt::AlignRight);
            row[COL_TOTAL]->setTextAlignment(Qt::AlignRight);
            row[COL_PRICE]->setData(0,USER_ROLE_SORT_NUMERIC);
            row[COL_TOTAL]->setData(0,USER_ROLE_SORT_NUMERIC);
            _model->appendRow(row);
            _rows.insert(key,row);
        }
        else
        {
            untouchedRows.remove(key);
        }
        if(asset)
        {
            item->setData(asset->id(),ITEMDATA_ASSET_ID);
            double      price       =1.0f;
            cmc->assetPrice(cmc->quoteAssetID(),asset->id(),price,nullptr);
            double total=price*h.totalAmount;
            row[COL_NAME]->setText(asset?asset->name():"");
            if(row[COL_NAME]->icon().isNull())
            {
                row[COL_NAME]->setIcon(cmc->assetIcon(asset->id()));
            }
            row[COL_PRICE]->setText(cmc->formatFiatCurrency(price,cmc->quoteAssetID()));
            row[COL_PRICE]->setData(price,USER_ROLE_SORT_NUMERIC);
            row[COL_TOTAL]->setText(_privateData(cmc->formatFiatCurrency(total,cmc->quoteAssetID())));
            row[COL_TOTAL]->setData(total,USER_ROLE_SORT_NUMERIC);

            double btc=h.totalAmount;
            if(cmc->assetPrice(md::Asset::BTC,h.assetID,btc,nullptr))
            {
                btcAmount+=btc;
            }
        }
        row[COL_AMOUNT]->setText(_privateData(cmc->formatAmount(h.totalAmount,8)));
        row[COL_AMOUNT]->setData(h.totalAmount,USER_ROLE_SORT_NUMERIC);
    }
    // remove untouched rows
    foreach(QString ukey,untouchedRows.keys())
    {
        Row untouched=untouchedRows[ukey];
        QModelIndex index=_model->indexFromItem(untouched[0]);
        _model->removeRow(index.row());
        _rows.remove(ukey);
    }

    //
    {
        QString rt;
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
        rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg(tr("Total"));
        //rt+="<br>";
        rt+=QString("<span style=\"font-size:11pt;\"><b>&nbsp;%1 </b></span>").arg(_privateData(cmc->formatFiatCurrency(btcAmount,md::Asset::BTC)));
        //rt+=QString("<span style=\"font-size:9pt;\">&nbsp;%1</span>").arg(_currencyTicker);
        rt+="</div>";
        _labelTotalBalance->setText(rt);
    }
    {
        QString rt;
        double  price=btcAmount;
        cmc->assetPrice(cmc->quoteAssetID(),md::Asset::BTC,price,nullptr);
        QString fiatPriceStr=cmc->formatFiatCurrency(price,cmc->quoteAssetID());
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
        rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("&nbsp;");
        //rt+="<br>";
        rt+=QString("<span style=\"font-size:11pt;\">&nbsp;%1</span>").arg(_privateData(fiatPriceStr));
        rt+="</div>";
        _labelTotalBalanceQuote->setText(rt);
    }
}

//
void                        AssetsHoldingsWidget::onUpdate()
{
    refreshHoldings();
}

//
void                        AssetsHoldingsWidget::onWalletsChanged()
{
    _upToDate=false;
}

//
void                        AssetsHoldingsWidget::onCurrentRowChanged(QModelIndex index,QModelIndex /*oldIndex*/)
{
    QModelIndex             firstIndex  =index.sibling(index.row(),0);
    QString                 ticker      =firstIndex.data().toString();
    QString                 assetID     =firstIndex.data(ITEMDATA_ASSET_ID).toString();
    if(assetID.length())
    {
        TM_TRACE(TAG,QString("Activate ticker index %1 (%2)").arg(index.row()).arg(assetID));
        Application::instance().mainWindow()->openAssetProperties(assetID);
    }
    else
    {
        TM_WARNING(TAG,QString("No asset ID: %1").arg(assetID));
    }
}

