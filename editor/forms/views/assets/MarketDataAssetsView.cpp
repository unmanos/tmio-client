#include "Application.h"
#include "MainWindow.h"
#include "MarketDataAssetsView.h"
#include "libs/ui/sort/GenericSortFilterProxyModel.h"
#include "forms/admin/MarketDataEditAssetDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTimer>
#include <QToolButton>
#include <QMessageBox>

#include <md/MarketData.h>
#include <md/Asset.h>

#define TAG "views.CoinMarketCapView"
#define ITEMDATA_ASSET_ID       (Qt::UserRole+4)
#define ITEMDATA_CMC_LINK       (Qt::UserRole+5)

//
#define COL_RANK                0
#define COL_NAME                1
#define COL_SYMBOL              2
#define COL_PRICE               3
#define COL_PRICE_TIME          4
#define COL_MARKET_CAP          5
#define COL_CIRCULATING_SUPPLY  6
//#define COL_CHANGE_1H           6
//#define COL_CHANGE_1D           7
//#define COL_CHANGE_1W           8
#define COL_COUNT               7

//
MarketDataAssetsView::MarketDataAssetsView(QWidget *parent) : QWidget(parent)
{
    md::MarketData*     marketData=Application::instance().marketData();
    // layout
    setLayout(new QVBoxLayout());
    setWindowTitle(tr("Assets list"));
    // title frame
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    QFrame*             titleFrame=new QFrame();
    titleFrame->setLayout(topFormLayout);
    topFormLayout->setMargin(0);
    topFormLayout->addSpacing(8);
    //titleFrame->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    layout()->addWidget(titleFrame);
    // add coin icon
    {
        QLabel*             coinIcon=new QLabel();
        coinIcon->setPixmap(QIcon(":/icons/assets.png").pixmap(30,30));
        topFormLayout->addWidget(coinIcon);
    }
    // add market info
    {
        _labelMarketInfo= new QLabel();
        topFormLayout->addWidget(_labelMarketInfo);
        topFormLayout->addStretch(1);
    }
    // add market cap
    {
        _labelMarketCap= new QLabel();
        topFormLayout->addWidget(_labelMarketCap);
        topFormLayout->addStretch(1);
    }
    // add BTC dominance
    {
        _labelBtcDominance= new QLabel();
        topFormLayout->addWidget(_labelBtcDominance);
        topFormLayout->addStretch(1);
    }
    // add filter
    {
        _filterTypeCombo    =new QComboBox(this);
        _filterTypeCombo->addItem("crypto","currency+token");
        _filterTypeCombo->addItem("fiat","fiat");
        topFormLayout->addWidget(_filterTypeCombo);
        connect(_filterTypeCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onFilterTypeChanged(int)));
    }
    // add admin tools
    if(Application::instance().connection().account()->isMemberOf("marketdata"))
    {
        // create asset button
        _createButton=new QToolButton(this);
        _createButton->setText("Create asset");
        topFormLayout->addWidget(_createButton);
        connect(_createButton,&QToolButton::clicked,this,&MarketDataAssetsView::onCreateAsset);
        // delete asset button
        _deleteButton=new QToolButton(this);
        _deleteButton->setText("Delete asset");
        topFormLayout->addWidget(_deleteButton);
        connect(_deleteButton,&QToolButton::clicked,this,&MarketDataAssetsView::onDeleteAsset);
        _deleteButton->setEnabled(false);
    }


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
    _model->setHorizontalHeaderItem(COL_PRICE_TIME, new QStandardItem(tr("Updated")));
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
    //_treeView->setModel(_model);

    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(true);
    _treeView->setHeader(listHeader);
    listHeader->setSortIndicator(COL_MARKET_CAP,Qt::SortOrder::DescendingOrder);

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
    _treeView->setSelectionBehavior(QAbstractItemView::SelectRows);

    _treeView->setUniformRowHeights(true);

    layout()->addWidget(_treeView);

    QTimer*     timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(onUpdateData()));
    timer->start(1000);

    //
    updateData();


    // select current asset
    const QString               currentAssetID  =Application::instance().mainWindow()->currentAssetProperties();
    Row                         row             =_lookup.contains(currentAssetID)?_lookup[currentAssetID]:Row();
    if(row.size())
    {
        QModelIndex     realIndex=_sortFilter->mapFromSource(row[0]->index());
        _treeView->selectionModel()->setCurrentIndex(realIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        TM_TRACE(TAG,QString("Found current asset '%1', selected.").arg(currentAssetID));
    }

    connect(_treeView->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));
    connect(_treeView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(_treeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onDoubleClickedAsset(QModelIndex)));
    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));
    connect(marketData,&md::MarketData::assetModified,this,&MarketDataAssetsView::onAssetModified);
    connect(marketData,&md::MarketData::assetDeleted,this,&MarketDataAssetsView::onAssetDeleted);
}

//
QString         MarketDataAssetsView::friendlyTime(const QDateTime& t)
{
    qint64      secs=t.secsTo(QDateTime::currentDateTime());
    qint64      minutes=secs/60;
    qint64      hours=minutes/60;
    qint64      days=hours/24;
    if(secs<15)
        return("now");
    if(secs<60)
        return("recently");
    if(minutes<60)
    {
        return(QString("%1 minute%2 ago").arg(minutes).arg((minutes!=1)?"s":""));
    }
    if(hours<24)
    {
        return(QString("%1 hour%2 ago").arg(hours).arg((hours!=1)?"s":""));
    }
    return(QString("%1 day%2 ago").arg(days).arg((days!=1)?"s":""));
}

//
void            MarketDataAssetsView::updateData()
{
    md::MarketData*     md      =Application::instance().marketData();
    QList<QString>      assets  =md->assets();
    double                  totalMarketCap  =0;
    double                  bitcoinCap      =0;
    QStringList             filterType      =_filterTypeCombo->currentData().toString().split("+");
    int                     assetCount      =0;
    CoinMarketCap*          cmc             =Application::instance().cmc();
    foreach(QString assetId,assets)
    {
        md::Asset*                  asset=md->asset(assetId);
        if(!filterType.contains(asset->type()))
                continue;
        QStandardItem*              item=_lookup.contains(assetId)?_lookup[assetId].first():nullptr;
        Row                         row;
        if(!item)
        {
            row.reserve(COL_COUNT);
            for(int i=0;i<COL_COUNT;i++)
            {
                QStandardItem*   rowItem=new QStandardItem("?");
                row.append(rowItem);
                if(i==0)
                {
                    item=rowItem;
                }
            }
            row[COL_PRICE]->setTextAlignment(Qt::AlignRight);
            row[COL_MARKET_CAP]->setTextAlignment(Qt::AlignRight);
            row[COL_CIRCULATING_SUPPLY]->setTextAlignment(Qt::AlignRight);
            row[COL_RANK]->setData(assetId,ITEMDATA_ASSET_ID);
            _model->appendRow(row.toList());
            _lookup[assetId]=row;
        }
        else
        {
            row=_lookup[assetId];
        }
        row[COL_NAME]->setText(asset->name());
        row[COL_SYMBOL]->setText(asset->symbol());
        // update row
        double circulatingSupply    =cmc->circulatingSupply(asset->id());

        double price                =1;
        QDateTime                   updateTime;
        if(cmc->assetPrice(cmc->quoteAssetID(),asset->id(),price,nullptr))
        {
            updateTime=asset->lastPriceUpdate();
            row[COL_PRICE]->setText(cmc->formatFiatCurrency(price,cmc->quoteAssetID()));
            row[COL_PRICE]->setData(price,USER_ROLE_SORT_NUMERIC);
            if(circulatingSupply>0.001)
            {
                double marketCap            =circulatingSupply*price;
                row[COL_MARKET_CAP]->setText(cmc->formatFiatCurrency(marketCap,cmc->quoteAssetID()));
                row[COL_MARKET_CAP]->setData(marketCap,USER_ROLE_SORT_NUMERIC);
                totalMarketCap+=marketCap;
                if(asset->id()==md::Asset::BTC)
                    bitcoinCap=marketCap;
            }
            else
            {
                row[COL_MARKET_CAP]->setText("?");
                row[COL_MARKET_CAP]->setData(static_cast<double>(0),USER_ROLE_SORT_NUMERIC);
            }
        }
        else
        {
            row[COL_PRICE]->setText("?");
            row[COL_PRICE]->setData(static_cast<double>(0),USER_ROLE_SORT_NUMERIC);
            row[COL_MARKET_CAP]->setText("?");
            row[COL_MARKET_CAP]->setData(static_cast<double>(0),USER_ROLE_SORT_NUMERIC);
        }
        int             rank=cmc->assetRank(asset->id());
        if(rank)
        {
            row[COL_RANK]->setText(rank?QString::number(rank):"?");
            row[COL_RANK]->setData(static_cast<double>(rank),USER_ROLE_SORT_NUMERIC);
        }
        else
        {
            row[COL_RANK]->setText("?");
        }

        if(!updateTime.isNull())
        {
            row[COL_PRICE_TIME]->setText(friendlyTime(updateTime));
            row[COL_PRICE_TIME]->setData(static_cast<double>(updateTime.toSecsSinceEpoch()),USER_ROLE_SORT_NUMERIC);
        }
        else
        {
            row[COL_PRICE_TIME]->setText("?");
            row[COL_PRICE_TIME]->setData(static_cast<double>(0),USER_ROLE_SORT_NUMERIC);
        }
        if(row[COL_NAME]->icon().isNull())
        {
            const QRect visualRect=_treeView->visualRect(_sortFilter->mapFromSource(item->index()));
            const bool visible=(visualRect.top()>=0)&&(visualRect.bottom()<=height());
            if(visible)
                row[COL_NAME]->setIcon(Application::instance().cmc()->assetIcon(asset->id()));
        }
        if(circulatingSupply)
        {
            row[COL_CIRCULATING_SUPPLY]->setText(cmc->formatSupply(circulatingSupply));
            row[COL_CIRCULATING_SUPPLY]->setData(circulatingSupply,USER_ROLE_SORT_NUMERIC);
        }
        else
        {
            row[COL_CIRCULATING_SUPPLY]->setData(0,USER_ROLE_SORT_NUMERIC);
        }
        assetCount++;
    }
    // market info
    {
        QString rt;
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;\">";
        rt+=QString("<span style=\"font-size:11pt;\"><b>%1</b></span>").arg(assetCount);
        rt+=QString("<span style=\"font-size:9pt;\"> %1</span>").arg("assets");
        rt+="<br>";
        rt+=QString("<span style=\"font-size:8pt;\"><b>%1</b> exchanges</span>").arg(md->exchanges().count());
        rt+="</div>";
        _labelMarketInfo->setText(rt);
    }
    // market cap
    {
        QString rt;
        if(!filterType.contains("fiat"))
        {
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("Market capitalisation");
            rt+="<br>";
            rt+=QString("<span style=\"font-size:11pt;\"><b>%1 </b></span>").arg(cmc->formatFiatCurrency(totalMarketCap,cmc->quoteAssetID()));
            //rt+=QString("<span style=\"font-size:9pt;\">%1</span>").arg(_quoteTicker);
            rt+="</div>";
        }
        _labelMarketCap->setText(rt);
    }
    // BTC dominance
    {
        QString rt;
        if( (!filterType.contains("fiat"))&&(totalMarketCap>0.0001/*quantum computing came?*/) )
        {
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("BTC Dominance");
            rt+="<br>";
            rt+=QString("<span style=\"font-size:11pt;\"><b>%1% </b></span>").arg(QString::number((bitcoinCap/totalMarketCap)*100.0,'f',1));
            //rt+=QString("<span style=\"font-size:9pt;\">%1</span>").arg(_quoteTicker);
            rt+="</div>";
        }
        _labelBtcDominance->setText(rt);
    }
}

//
bool            MarketDataAssetsView::loadSources()
{
    Application::instance().marketData()->loadSources();
    if(!Application::instance().marketData()->sourcesReady())
    {
        QMessageBox::critical(this,"Sources not ready","Sources are still loading loaded, please try again in a few seconds.");
        return(false);
    }
    return(true);
}

//
void            MarketDataAssetsView::onFilterTypeChanged(int /*index*/)
{
    if(_model->hasChildren())
    {
        _model->removeRows(0, _model->rowCount());
    }
    _lookup.clear();
    updateData();
}

//
void            MarketDataAssetsView::onUpdateData()
{
    if(_treeView->isVisible())
    {
        updateData();
    }
}

//
void            MarketDataAssetsView::onQuoteAssetChanged()
{
    updateData();
}

//
void            MarketDataAssetsView::onCurrentRowChanged(QModelIndex index,QModelIndex oldIndex)
{
    QModelIndex             firstIndex=index.sibling(index.row(),0);
    QString                 ticker=firstIndex.data().toString();
    QString                 assetId=firstIndex.data(ITEMDATA_ASSET_ID).toString();
    if(!assetId.isEmpty())
    {
        if(oldIndex.isValid())
        {
            TM_TRACE(TAG,QString("Activate ticker index %2 (%3)").arg(index.row()).arg(ticker));
            Application::instance().mainWindow()->openAssetProperties(assetId);
        }
    }
    else
    {
    }
}

//
void            MarketDataAssetsView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& /*oldIndex*/)
{
    if(_deleteButton)
    {
        _deleteButton->setEnabled(selected.indexes().count()!=0);
    }
}

//
void            MarketDataAssetsView::onDoubleClickedAsset(QModelIndex index)
{
    if(!loadSources())
    {
        return;
    }
    if(Application::instance().connection().account()->isMemberOf("marketdata"))
    {
        QModelIndex             firstIndex=index.sibling(index.row(),0);
        QString                 ticker=firstIndex.data().toString();
        QString                 assetId=firstIndex.data(ITEMDATA_ASSET_ID).toString();
        if(!assetId.isEmpty())
        {
            TM_TRACE(TAG,QString("Double clicked ticker index %2 (%3)").arg(index.row()).arg(assetId));
            //Application::instance().mainWindow()->openCoinProperties(cmcId);
            MarketDataEditAssetDialog     dlg(assetId,this);
            dlg.setModal(true);
            if(dlg.exec()==QDialog::Accepted)
            {

            }
        }
    }
}

//
void            MarketDataAssetsView::onCreateAsset(bool)
{
    if(!loadSources())
    {
        return;
    }
    TM_TRACE(TAG,"Create asset");
    MarketDataEditAssetDialog     dlg("",this);
    dlg.setModal(true);
    if(dlg.exec()==QDialog::Accepted)
    {

    }
}

//
void            MarketDataAssetsView::onDeleteAsset(bool)
{
    if(!loadSources())
    {
        return;
    }
    QModelIndexList     selection   =_treeView->selectionModel()->selectedIndexes();
    md::MarketData*     marketData  =Application::instance().marketData();
    if(selection.length()!=0)
    {
        QString                 assetId =selection.first().data(ITEMDATA_ASSET_ID).toString();
        if(!assetId.isEmpty())
        {
            md::Asset*          asset   =marketData->asset(assetId);
            if(asset)
            {
                //QMessageBox::question(this,"Error saving asset","Asset ID must be unique and non empty.");
                int ret = QMessageBox::warning(this, tr("Confirm asset deletion ?"),
                                               QString(tr("Are you sure you want to delete asset '%1' ?").arg(asset->name())),
                                               QMessageBox::Yes | QMessageBox::No);
                if(ret==QMessageBox::Yes)
                {
                    TM_TRACE(TAG,QString("Delete asset '%1'").arg(asset->name()));
                    if(marketData->deleteAsset(asset->id()))
                    {
                        QMessageBox::information(this,tr("Asset deleted"),QString(tr("Asset '%1' have been deleted.").arg(asset->name())));
                    }
                    else
                    {
                        QMessageBox::critical(this,"Error deleting asset","Asset could not be deleted.");
                    }
                }
            }
        }
    }
}

//
void            MarketDataAssetsView::onAssetModified(const QString& id)
{
    if(_lookup.contains(id))
    {
        Row                         row=_lookup.contains(id)?_lookup[id]:Row();
        if(Application::instance().connection().account()->isMemberOf("marketdata"))
        {
            row[COL_NAME]->setIcon(QIcon());
        }
    }
    updateData();
}

//
void            MarketDataAssetsView::onAssetDeleted(const QString& id)
{
    if(_lookup.contains(id))
    {
        Row                         row=_lookup[id];
        _model->removeRow(row[0]->row());
    }
}
