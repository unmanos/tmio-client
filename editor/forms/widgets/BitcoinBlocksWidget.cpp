#include "Application.h"
#include "BitcoinBlocksWidget.h"
#include "libs/ui/sort/GenericSortFilterProxyModel.h"
#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>

//
#define COL_HEIGHT              0
#define COL_TIME                1
#define COL_HASH                2
#define COL_NTX                 3
#define COL_REWARD              4
#define COL_COUNT               5

//
#define TAG                     "BitcoinBlocksWidget"
#define URL_BLOCKS              "blockchains/bitcoin/blocks"

//
BitcoinBlocksWidget::BitcoinBlocksWidget()
{
    setLayout(new QVBoxLayout());

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
    _model->setHorizontalHeaderItem(COL_HEIGHT, new QStandardItem(tr("Height")));
    _model->setHorizontalHeaderItem(COL_TIME, new QStandardItem(tr("Time")));
    _model->setHorizontalHeaderItem(COL_HASH, new QStandardItem(tr("Hash")));
    _model->setHorizontalHeaderItem(COL_NTX, new QStandardItem(tr("Transactions")));
    _model->setHorizontalHeaderItem(COL_REWARD, new QStandardItem(tr("Reward")));

    sortFilter->setSourceModel(_model);
    treeView->setModel(sortFilter);

    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(true);
    treeView->setHeader(listHeader);
    listHeader->setSortIndicator(COL_HEIGHT,Qt::SortOrder::DescendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->resizeSection(COL_HEIGHT,100);
    listHeader->resizeSection(COL_TIME,150);
    listHeader->resizeSection(COL_HASH,500);
    listHeader->resizeSection(COL_NTX,100);
    listHeader->resizeSection(COL_REWARD,100);

    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setUniformRowHeights(true);

    layout()->addWidget(treeView);

    connect(Application::instance().marketData(),&md::MarketData::assetModified,this,&BitcoinBlocksWidget::onAssetModified);
    onRequestBlocks();
}

//
void                    BitcoinBlocksWidget::updateBlock(QJsonObject block)
{
    Row         row;
    const int   height  =block["_id"].toInt();
    if(!_rows.contains(height))
    {
        for(int i=0;i<COL_COUNT;i++)
        {
            row.append(new QStandardItem());
        }
        _model->appendRow(row);
        _rows.insert(height,row);
    }
    else
    {
        row=_rows[height];
    }
    row[COL_HEIGHT]->setText(QString("%1").arg(height));
    row[COL_HEIGHT]->setData(height,USER_ROLE_SORT_NUMERIC);

    row[COL_NTX]->setText(QString("%1").arg(block["ntx"].toInt()));
    row[COL_NTX]->setData(block["ntx"].toInt(),USER_ROLE_SORT_NUMERIC);

    row[COL_HASH]->setText(block["hash"].toString());
    row[COL_TIME]->setText(block["time"].toString());
    row[COL_REWARD]->setText(QString::number(block["reward"].toDouble()));
}

//
void                    BitcoinBlocksWidget::onAssetModified(const QString& id)
{
    if(id=="bitcoin")
    {
        onRequestBlocks();
    }
}

//
void                    BitcoinBlocksWidget::onRequestBlocks()
{
    if(_blocksRequest==nullptr)
    {
        _blocksRequest=Application::instance().connection().createRequest(URL_BLOCKS);
        connect(_blocksRequest, &tmio::rest::Request::finished, this, &BitcoinBlocksWidget::onReceiveBlocks);
        _blocksRequest->get(false);
    }
}

//
void                    BitcoinBlocksWidget::onReceiveBlocks(tmio::rest::RequestResult& result)
{
    TM_TRACE(TAG,"received blocks");
    if(result._statusCode==200)
    {
        QJsonArray          jsonArray=result._document.array();
        for(int row=0;row<jsonArray.count();row++)
        {
            QJsonObject         jsBlock         =jsonArray.at(row).toObject();
            updateBlock(jsBlock);
        }
    }
    delete(_blocksRequest);
    _blocksRequest=nullptr;
}
