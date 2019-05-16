#include <QFrame>
#include <QBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QHeaderView>

#include "am/Wallet.h"
#include "AssetsSummaryView.h"
#include "Application.h"
#include "MainWindow.h"

#include <md/Asset.h>
#include <md/Exchange.h>

//
#define TAG                     "AssetsSummaryView"

//
#define COL_ORDER               0
#define COL_TYPE                1
#define COL_EXCHANGE            2
#define COL_WALLET_NAME         3
#define COL_NAME                4
#define COL_SYMBOL              5
#define COL_UNIT_PRICE          6
#define COL_AMOUNT              7
#define COL_TOTAL               8
#define COL_COUNT               9

//
#define ITEMDATA_ASSET_ID       (Qt::UserRole+1)

//
AssetsSummaryView::AssetsSummaryView(QWidget *parent) : QWidget(parent)
{
    // layout
    setLayout(new QVBoxLayout());
    setWindowTitle(tr("Assets summary"));
    layout()->addWidget(buildTitleFrame());

    //QHBoxLayout*    viewLayout=new QHBoxLayout();
    QFrame*         viewFrame=new QFrame();
    viewFrame->setLayout(new QHBoxLayout());
    viewFrame->layout()->setSpacing(0);
    viewFrame->layout()->setContentsMargins(0,0,0,0);
    viewFrame->layout()->addWidget(buildTreeView());

    QFrame*         piesFrame=new QFrame();
    piesFrame->setLayout(new QVBoxLayout());

    _assetsPie          =new PieChart();
    _assetsPie->setMinimumWidth(384);
    piesFrame->layout()->addWidget(_assetsPie);
    _managerPie         =new PieChart();
    _managerPie->setMinimumWidth(384);
    piesFrame->layout()->addWidget(_managerPie);

    viewFrame->layout()->addWidget(piesFrame);

    layout()->addWidget(viewFrame);
    // signals
    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));
    connect(Application::instance().assetsManager(),&am::AssetsManager::walletsChanged,this,&AssetsSummaryView::onWalletsChanged);
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,SIGNAL(timeout()),this,SLOT(onUpdate()));
    updateTimer->start(1000*1/*10s update*/);
    // load avatar
    connect(Application::instance().connection().account(),SIGNAL(changed()),this,SLOT(onAccountChanged()));
    onAccountChanged();
    // first update
    onUpdate();
}

//
QWidget*                        AssetsSummaryView::buildTitleFrame()
{
    QFrame*             titleFrame=new QFrame();
    // title frame
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    titleFrame->setLayout(topFormLayout);
    topFormLayout->setMargin(0);
    topFormLayout->addSpacing(8);
    //titleFrame->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    // add coin icon
    {
        QLabel*             coinIcon=new QLabel();
        coinIcon->setPixmap(QIcon(":/icons/assets.png").pixmap(30,30));
        topFormLayout->addWidget(coinIcon);
        //topFormLayout->addStretch(1);
    }
    // add confirmed balance
    {
        _labelTotalBalance = new QLabel();
        _labelTotalBalanceFiat = new QLabel();
        topFormLayout->addWidget(_labelTotalBalance);
        topFormLayout->addSpacing(20);
        topFormLayout->addWidget(_labelTotalBalanceFiat);
        topFormLayout->addStretch(1);
    }
    // add pending balance
    {
        _labelAvailableBalance = new QLabel();
        _labelAvailableBalanceFiat = new QLabel();
        topFormLayout->addWidget(_labelAvailableBalance);
        topFormLayout->addSpacing(20);
        topFormLayout->addWidget(_labelAvailableBalanceFiat);
        topFormLayout->addStretch(1);
    }
    return(titleFrame);
}

//
QWidget*                        AssetsSummaryView::buildTreeView()
{
    _treeView           =new QTreeView(this);
    _treeView->setSortingEnabled(true);
    _treeView->setAlternatingRowColors(true);
    _treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _sortFilter         =new GenericSortFilterProxyModel(_treeView);
    _model              =new QStandardItemModel();
    //_model->setHorizontalHeaderItem(COL_RANK, new QStandardItem(tr("Rank")));
    _model->setHorizontalHeaderItem(COL_ORDER, new QStandardItem(tr("Order")));
    _model->setHorizontalHeaderItem(COL_TYPE, new QStandardItem(tr("Type")));
    _model->setHorizontalHeaderItem(COL_EXCHANGE, new QStandardItem(tr("Managed by")));
    _model->setHorizontalHeaderItem(COL_WALLET_NAME, new QStandardItem(tr("Wallet name")));
    _model->setHorizontalHeaderItem(COL_NAME, new QStandardItem(tr("Name")));
    _model->setHorizontalHeaderItem(COL_SYMBOL, new QStandardItem(tr("Symbol")));
    _model->setHorizontalHeaderItem(COL_UNIT_PRICE, new QStandardItem(tr("Unit price")));
    _model->setHorizontalHeaderItem(COL_AMOUNT, new QStandardItem(tr("Amount")));
    _model->setHorizontalHeaderItem(COL_TOTAL, new QStandardItem(tr("Total")));
    _sortFilter->setSourceModel(_model);
    _treeView->setModel(_sortFilter);
    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(false);
    _treeView->setHeader(listHeader);
    //listHeader->setSortIndicator(0,Qt::SortOrder::AscendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->hideSection(COL_ORDER);
    listHeader->resizeSection(COL_ORDER,200);
    listHeader->resizeSection(COL_TYPE,100);
    listHeader->resizeSection(COL_EXCHANGE,150);
    listHeader->resizeSection(COL_WALLET_NAME,180);
    listHeader->resizeSection(COL_NAME,180);
    listHeader->resizeSection(COL_SYMBOL,70);
    listHeader->resizeSection(COL_UNIT_PRICE,90);
    listHeader->resizeSection(COL_AMOUNT,110);
    listHeader->resizeSection(COL_TOTAL,110);
    _treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    //_treeView->setUniformRowHeights(true);
    _treeView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(_treeView->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));
    return(_treeView);
}

//
void                    AssetsSummaryView::updateUI()
{
#if 1
    md::MarketData*             md  =Application::instance().marketData();
    tmio::am::AssetsManager*    am  =Application::instance().assetsManager();
    CoinMarketCap*              cmc =Application::instance().cmc();
    //double amount[]={_confirmedBalance,_pendingBalance};

    // calcuate balance
    QMap<QString,Row>           untouchedRows   =_rows;
    QMap<QString,double>        perAssetHoldings;
    QMap<QString,double>        perManagerHoldings;
    QMap<QString,QIcon>         managersIcon;
    double                      btcAmount=0;
    QVector<tmio::am::Wallet*>  wallets=am->wallets();
    foreach(tmio::am::Wallet* wallet, wallets)
    {
        QVector<tmio::am::Wallet::Asset>      assets            =wallet->assets();
        QJsonObject                           walletConfig      =wallet->config();
        QString                               managerText       ="unknown";
        QIcon                                 managerIcon;
        QIcon                                 walletIcon;
        QString                               walletTypeText    ="unknown";
        QString                               walletType        ="unknown";
        //qDebug() << QJsonDocument(walletConfig).toJson();
        if(walletConfig["exchange"].isObject())
        {
            QJsonObject     exchangeConfig  =walletConfig["exchange"].toObject();
            const QString   exchangeID      =exchangeConfig.keys().first();
            md::Exchange*   exchange        =md->exchange(exchangeID);
            managerText                     =exchange?exchange->name():("unknown "+exchangeID);
            managerIcon                     =cmc->exchangeIcon(exchangeID);
            walletType                      ="exchange";
            walletTypeText                  ="Exchange";
            walletIcon                      =QIcon(":/icons/assets.png");
        }
        if(walletConfig["mining"].isObject())
        {
            QJsonObject     miningConfig    =walletConfig["mining"].toObject();
            const QString   miningID        =miningConfig.keys().first();
            managerText                     =cmc->miningPoolName(miningID);
            managerIcon                     =cmc->miningPoolIcon(miningID);
            walletType                      ="mining";
            walletTypeText                  ="Mining pool";
            walletIcon                      =QIcon(":/icons/mining.png");
        }
        if(walletConfig["private"].isObject())
        {
            managerText                     ="You";
            managerIcon                     =_avatarIcon;
            walletType                      ="private";
            walletTypeText                  ="Private";
            walletIcon                      =QIcon(":/icons/sync.png");
        }

        foreach(tmio::am::Wallet::Asset asset, assets)
        {
            const QString           key     =QString::number(wallet->id())+"."+asset.symbol;
            QList<QStandardItem*>   row     =_rows.contains(key)?_rows[key]:Row();
            QStandardItem*          item    =row.length()?row.at(0):nullptr;
            if(!item)
            {
                const QString               wtype=walletConfig.keys().first();
                item=new QStandardItem();
                row.append(item);
                for(int i=1;i<COL_COUNT;i++)
                {
                    row.append(new QStandardItem());
                }
                item->setText(key);
                row[COL_TYPE]->setText(walletTypeText);
                row[COL_TYPE]->setIcon(QIcon(":/icons/wallets/"+walletType+".png"));
                row[COL_EXCHANGE]->setText(managerText);
                row[COL_SYMBOL]->setText(asset.symbol);
                row[COL_UNIT_PRICE]->setTextAlignment(Qt::AlignRight);
                row[COL_AMOUNT]->setTextAlignment(Qt::AlignRight);
                row[COL_TOTAL]->setTextAlignment(Qt::AlignRight);
                _model->appendRow(row);
                _rows.insert(key,row);
            }
            else
            {
                untouchedRows.remove(key);
            }
            _model->item(item->row(),COL_AMOUNT)->setText(cmc->formatAmount(asset.totalAmount,8));
            _model->item(item->row(),COL_AMOUNT)->setData(asset.totalAmount,USER_ROLE_SORT_NUMERIC);
            _model->item(item->row(),COL_EXCHANGE)->setIcon(managerIcon);
            _model->item(item->row(),COL_WALLET_NAME)->setText(wallet->name());
            if(asset.assetID.length())
            {
                md::Asset*  mdAasset    =md->asset(asset.assetID);
                double      price       =1.0f;
                cmc->assetPrice(cmc->quoteAssetID(),asset.assetID,price,nullptr);
                double total=price*asset.totalAmount;
                row[COL_NAME]->setText(mdAasset?mdAasset->name():"");
                if(row[COL_NAME]->icon().isNull())
                {
                    row[COL_NAME]->setIcon(Application::instance().cmc()->assetIcon(asset.assetID));
                }
                row[COL_UNIT_PRICE]->setText(cmc->formatFiatCurrency(price,cmc->quoteAssetID()));
                row[COL_UNIT_PRICE]->setData(price,USER_ROLE_SORT_NUMERIC);
                row[COL_TOTAL]->setText(cmc->formatFiatCurrency(total,cmc->quoteAssetID()));
                row[COL_TOTAL]->setData(total,USER_ROLE_SORT_NUMERIC);

                //wallet->name()
                double btc=asset.totalAmount;
                if(cmc->assetPrice(md::Asset::BTC,asset.assetID,btc,nullptr))
                {
                    btcAmount+=btc;
                    if(!perAssetHoldings.contains(asset.assetID))
                        perAssetHoldings[asset.assetID]=0;
                    perAssetHoldings[asset.assetID]+=btc;
                    if(!perManagerHoldings.contains(managerText))
                    {
                        perManagerHoldings[managerText]=0;
                        managersIcon[managerText]=managerIcon;
                    }
                    perManagerHoldings[managerText]+=btc;
                }
            }
            item->setData(asset.assetID,ITEMDATA_ASSET_ID);
        }
    }
    // remove untouched rows
    foreach(QString ukey,untouchedRows.keys())
    {
        Row untouched=untouchedRows[ukey];
        QModelIndex index=_model->indexFromItem(untouched[0]);
        _model->removeRow(index.row());
        _rows.remove(ukey);
    }
    // update per-asset pie
    {
        _assetsPie->resetValues();
        double          smallAmounts=0;
        foreach(QString assetID,perAssetHoldings.keys())
        {
            const double            amount=perAssetHoldings[assetID];
            if(amount>=(btcAmount*0.05))
            {
                md::Asset*  mdAasset    =md->asset(assetID);
                _assetsPie->setValue(assetID,mdAasset->name(),mdAasset->icon(),perAssetHoldings[assetID]);
            }
            else
            {
                smallAmounts+=amount;
            }
        }
        _assetsPie->setValue("$","Others",smallAmounts);
    }
    // update per-manager pie
    {
        _managerPie->resetValues();
        double          smallAmounts=0;
        foreach(QString managerText,perManagerHoldings.keys())
        {
            const double            amount=perManagerHoldings[managerText];
            if(amount>=(btcAmount*0.05))
            {
                _managerPie->setValue(managerText,managerText,managersIcon[managerText],amount);
            }
            else
            {
                smallAmounts+=amount;
            }
        }
        _managerPie->setValue("$","Others",smallAmounts);
    }


    double amount[]={btcAmount,0};
    QString text[]={tr("Confirmed balance"),tr("Pending balance")};
    QLabel* labels[]={_labelTotalBalance,_labelAvailableBalance};
    QLabel* labelsFiat[]={_labelTotalBalanceFiat,_labelAvailableBalanceFiat};
    for(int i=0;i<2;i++)
    {
        {
            QString rt;
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg(text[i]);
            rt+="<br>";
            rt+=QString("<span style=\"font-size:11pt;\"><b>%1 </b></span>").arg(cmc->formatFiatCurrency(amount[i],"bitcoin"));
            //rt+=QString("<span style=\"font-size:9pt;\">%1</span>").arg(_currencyTicker);
            rt+="</div>";
            labels[i]->setText(rt);
        }
        {
            QString rt;
            double  price=amount[i];
            cmc->assetPrice(cmc->quoteAssetID(),md::Asset::BTC,price,nullptr);
            QString fiatPriceStr=cmc->formatFiatCurrency(price,cmc->quoteAssetID());
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("&nbsp;");
            rt+="<br>";
            rt+=QString("<span style=\"font-size:11pt;\">%1</span>").arg(fiatPriceStr);
            rt+="</div>";
            labelsFiat[i]->setText(rt);
        }
    }
#endif
    _upToDate       =true;
}

//
void                    AssetsSummaryView::onUpdate()
{
    updateUI();
}

//
void                    AssetsSummaryView::onQuoteAssetChanged()
{
    updateUI();
}

//
void                    AssetsSummaryView::onCurrentRowChanged(QModelIndex index,QModelIndex /*oldIndex*/)
{
    QModelIndex             firstIndex  =index.sibling(index.row(),0);
    QString                 ticker      =firstIndex.data().toString();
    QString                 assetID     =firstIndex.data(ITEMDATA_ASSET_ID).toString();
    if(assetID.length())
    {
        TM_TRACE(TAG,QString("Activate ticker index %1 (%2)").arg(index.row()).arg(assetID));
        Application::instance().mainWindow()->openAssetProperties(assetID);
        //TM_TRACE(TAG,"TODO: AssetsSummaryView::onCurrentRowChanged");
    }
    else
    {
        TM_WARNING(TAG,QString("No asset ID: %1").arg(assetID));
    }
}

//
void                    AssetsSummaryView::onWalletsChanged()
{
    /*TM_TRACE(TAG,"Wallets changed");*/
    _upToDate           =false;
}

//
void                    AssetsSummaryView::onAccountChanged()
{
    tmio::rest::Connection&   conn    =Application::instance().connection();
    _avatarIcon=QIcon();
    if(conn.account()->avatarImage().length())
    {
        QImage image;
        image.loadFromData(conn.account()->avatarImage());
        QPixmap buffer = QPixmap::fromImage(image);
        _avatarIcon.addPixmap(buffer);
    }
}
