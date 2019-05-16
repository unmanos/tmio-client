#include <QStandardItem>
#include <QTableView>
#include <QCompleter>
#include <QMessageBox>
#include <QHeaderView>
#include "MarketDataEditAssetDialog.h"
#include "ui_MarketDataEditAssetDialog.h"
#include "Application.h"

#include <md/Asset.h>
#include <md/Exchange.h>

//
#define TAG                     "MarketDataEditAssetDialog"
#define PROPERTY_EXCHANGE_ID    "ExchangeID"
#define PROPERTY_SOURCE_NAME    "SourceName"

//
#define SOURCE_URL              "marketdata/sources/%1/coins?idasstring=true"
#define SOURCE_ICON_URL         "marketdata/sources/%1/coins/%2/icon"

//
MarketDataEditAssetDialog::MarketDataEditAssetDialog(const QString& assetID,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MarketDataEditAssetDialog)
{
    ui->setupUi(this);
    _assetID            =assetID;
    _createAsset        =assetID.isEmpty();
    setWindowTitle(_createAsset?tr("Create new asset..."):tr("Edit asset..."));

    md::MarketData*     marketData  =Application::instance().marketData();
    md::Asset*          asset       =marketData->asset(assetID);
    ui->assetType->addItem("currency");
    ui->assetType->addItem("token");
    ui->assetType->addItem("fiat");
    if(asset)
    {
        ui->assetType->setCurrentText(asset->type());
    }
    loadAsset();
    foreach(QString sourceName,marketData->availableSources())
    {
        // build source
        Source              source;
        source.name         =sourceName;
        if(sourceName==md::sourceCoinmarketcap)
        {
            source.combo            =ui->cmcComboCoins;
            source.filter           =ui->cmcFilterCoins;
            source.circulatingSupply=ui->cmcCirculatingSupply;
            source.priceBtc         =ui->cmcBtcPrice;
            source.identifier       =ui->cmcIdentifier;
            source.symbol           =ui->cmcSymbol;
        }
        if(sourceName==md::sourceCoingecko)
        {
            source.combo            =ui->geckoComboCoins;
            source.filter           =ui->geckoFilterCoins;
            source.circulatingSupply=ui->geckoCirculatingSupply;
            source.priceBtc         =ui->geckoBtcPrice;
            source.identifier       =ui->geckoIdentifier;
            source.symbol           =ui->geckoSymbol;
        }
        source.combo->setProperty(PROPERTY_SOURCE_NAME,sourceName);
        source.filter->setProperty(PROPERTY_SOURCE_NAME,sourceName);
        if(asset)
        {
            source.filter->setText(asset->name());
            source.selectedCoinID=asset->source(source.name);
        }
        source.priceBtc->setReadOnly(true);
        source.circulatingSupply->setReadOnly(true);
        source.symbol->setReadOnly(true);
        source.identifier->setReadOnly(true);
        _sources.insert(source.name,source);
        // refresh UI
        refreshCombo(source);
        refreshSourceData(source);
        // connect UI logic
        connect(source.filter,&QLineEdit::textChanged,this,&MarketDataEditAssetDialog::onSourceFilterChanged);
        //connect(source.combo,SIGNAL(currentIndexChanged(int)),this,SLOT(onSourceCoinChanged(int)));
    }
    // exchanges
    QFormLayout*    exchangesLayout=new QFormLayout(ui->exchanges);
    foreach(QString exchangeID,marketData->exchanges())
    {
        QWidget*        label       =new QWidget();
        label->setLayout(new QHBoxLayout());
        md::Exchange*   exchange    =marketData->exchange(exchangeID);
        QLabel*         labelIcon   =new QLabel();
        labelIcon->setPixmap(Application::instance().cmc()->exchangeIcon(exchangeID).pixmap(16,16));
        QLabel*         labelText   =new QLabel(exchange->name());
        label->layout()->addWidget(labelIcon);
        label->layout()->addWidget(labelText);
        QComboBox*      combo   =new QComboBox();
        combo->addItem("(none)");
        foreach(const QString& symbol,exchange->unregisteredSymbols())
        {
            combo->addItem(symbol,symbol);
        }
        exchangesLayout->addRow(label,combo);
        if(asset)
        {
            const QString currentSymbol=asset->exchange(exchangeID);
            if(!currentSymbol.isEmpty())
            {
                if(combo->findData(currentSymbol)<0)
                {
                    combo->addItem(currentSymbol,currentSymbol);
                }
                combo->setCurrentText(currentSymbol);
                _exchanges[exchangeID]=currentSymbol;
            }
        }
        combo->setProperty(PROPERTY_EXCHANGE_ID,exchangeID);
        connect(combo,SIGNAL(currentIndexChanged(int)),this,SLOT(onAssetExchangeChanged(int)));
    }

    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(onSave()));
    connect(ui->assetType,SIGNAL(currentIndexChanged(int)),this,SLOT(onAssetTypeChanged(int)));
}

//
MarketDataEditAssetDialog::~MarketDataEditAssetDialog()
{
    delete ui;
}

//
void                            MarketDataEditAssetDialog::accept()
{
    TM_TRACE(TAG,"onSave");
    md::MarketData*     marketData  =Application::instance().marketData();
    md::Asset*          asset       =nullptr;
    if(!_createAsset)
    {
        asset           =marketData->asset(_assetID);
    }
    else
    {
        // create asset
        //_assetID
        _assetID    =ui->assetID->text();
        asset       =marketData->createAsset(_assetID);
        if(!asset)
        {
            QMessageBox::critical(this,"Error saving asset","Asset ID must be unique and non empty.");
            return;
        }
    }
    if(asset)
    {
        foreach(const Source& source,_sources.values())
        {
            TM_TRACE(TAG,QString("Source %1 > %2").arg(source.name).arg(source.selectedCoinID));
            asset->setSource(source.name,source.selectedCoinID);
        }
        foreach(const QString& exchange,_exchanges.keys())
        {
            TM_TRACE(TAG,QString("Exchange %1 > %2").arg(exchange).arg(_exchanges[exchange]));
            asset->setExchange(exchange,_exchanges[exchange]);
        }
        asset->setName(ui->assetName->text());
        asset->setSymbol(ui->assetSymbol->text());
        asset->setType(ui->assetType->currentText());
        if(marketData->saveAsset(_assetID))
        {
            QDialog::accept();
            if(_createAsset)
            {
                // remove it straight after, it should be added automatically later
            }
        }
        else
        {
            QMessageBox::critical(this,"Error saving asset","The asset could not be saved to database.");
        }
    }
    else
    {
        QMessageBox::critical(this,"Error saving asset","The asset could not be created or was deleted during edition.");
    }
}

//
void                            MarketDataEditAssetDialog::loadAsset()
{
    md::MarketData*     marketData  =Application::instance().marketData();
    md::Asset*          asset       =marketData->asset(_assetID);
    if(asset)
    {
        ui->assetType->setCurrentText(asset->type());
        ui->assetID->setText(asset->id());
        ui->assetID->setEnabled(false);
        ui->assetSymbol->setText(asset->symbol());
        ui->assetName->setText(asset->name());
    }
    ui->tabWidget->setTabEnabled(0,ui->assetType->currentText()!="fiat");
    ui->tabWidget->setTabEnabled(1,ui->assetType->currentText()!="fiat");
}

//
void                            MarketDataEditAssetDialog::refreshCombo(Source& source)
{
    const QString               selectedCoinID=source.selectedCoinID;
    disconnect(source.combo,SIGNAL(currentIndexChanged(int)),this,SLOT(onSourceCoinChanged(int)));
    source.combo->clear();

    source.comboModel = new QStandardItemModel(this);
    source.combo->setAutoCompletion(true);
    //combo->setEditable(true);
    //ui->comboCmcCoins->sta
    //model->setRowCount(source.coins.count());
    const QString       filterText=source.filter->text();
    //const QString       selectedId=
    //md::Asset*          asset           =Application::instance().marketData()->asset(_assetID);
    md::Source*         dataSource      =Application::instance().marketData()->source(source.name);
    //const QString       selectedId      =asset->source(md::sourceName(source.id));
    QStandardItem*      selectedItem    =nullptr;
    source.comboModel->appendRow(new QStandardItem("(none)"));
    TM_TRACE(TAG,QString("Refresh combo, selected id: %1").arg(source.selectedCoinID));
    foreach(const md::Source::Coin& coin,dataSource->coins().values())
    {
        bool                match=false;
        if( (coin.name.contains(filterText,Qt::CaseInsensitive))||
            (coin.symbol.contains(filterText,Qt::CaseInsensitive))||
            (coin.id.contains(filterText,Qt::CaseInsensitive)) )
        {
            match=true;
        }
        if( (match)||(selectedCoinID==coin.id) )
        {
            QList<QStandardItem*>   row;
            row.append(new QStandardItem(coin.name));
            row.append(new QStandardItem(coin.symbol));
            row.append(new QStandardItem(coin.id));
            row[0]->setData(coin.id);
            row[1]->setData(coin.id);
            row[2]->setData(coin.id);
            row[0]->setIcon(coin.icon);
            source.comboModel->appendRow(row);
            if(selectedCoinID==coin.id)
            {
                TM_TRACE(TAG,QString("Refresh combo, selected coin found in list: %1").arg(source.selectedCoinID));
                selectedItem=row[0];
            }
        }
    }
    //source.comboModel->sort(0);
    //model->appendColumn()
    //model->

    // https://www.qtcentre.org/threads/23143-Combobox-entries-filter-as-I-type
    source.comboTree = new QTreeView(this); // create the tableview
    source.comboTree->setUniformRowHeights(true);
    source.combo->setView(source.comboTree); // set it to the comboBox before making changes
    source.comboTree->setSelectionMode(QAbstractItemView::SingleSelection);
    source.comboTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    source.combo->setModel(source.comboModel);
    source.comboTree->sortByColumn(0,Qt::SortOrder::AscendingOrder);
    source.comboTree->header()->hide();
    source.comboTree->setColumnWidth(0,250);
    source.comboTree->setColumnWidth(1, 50);
    source.comboTree->setColumnWidth(2,100);


    //coilView->selectionModel()->select(selectedIndex,QItemSelectionModel::SelectCurrent);
    //coilView->setCurrentIndex(model->index()
    if(selectedItem)
    {
        QModelIndex      realindex=source.comboModel->indexFromItem(selectedItem);
        source.combo->setCurrentIndex(realindex.row());

    }
    connect(source.combo,SIGNAL(currentIndexChanged(int)),this,SLOT(onSourceCoinChanged(int)));
}

//
void                            MarketDataEditAssetDialog::refreshSourceData(Source& source)
{
    md::MarketData*     marketData=Application::instance().marketData();
    md::Source*         marketSource=marketData->source(source.name);
    source.circulatingSupply->setText("NA");
    source.priceBtc->setText("NA");
    source.identifier->setText("NA");
    if(marketSource->coins().contains(source.selectedCoinID))
    {
        source.circulatingSupply->setText(QString::number(marketSource->coins()[source.selectedCoinID].circulatingSupply));
        source.priceBtc->setText(QString::number(marketSource->coins()[source.selectedCoinID].priceBTC,'f',8));
        source.identifier->setText(source.selectedCoinID);
        source.symbol->setText(marketSource->coins()[source.selectedCoinID].symbol);
    }
}

//
void                            MarketDataEditAssetDialog::onSourceFilterChanged(const QString& text)
{
    const QString               sourceName=sender()->property(PROPERTY_SOURCE_NAME).toString();
    TM_TRACE(TAG,QString("Filter text %1: %2").arg(sourceName).arg(text));
    refreshCombo(_sources[sourceName]);
}

//
void                            MarketDataEditAssetDialog::onSourceCoinChanged(int)
{
    const QString               sourceName  =sender()->property(PROPERTY_SOURCE_NAME).toString();
    Source&                     source      =_sources[sourceName];
    const QString           coinName=source.combo->currentText();
    const QString           coinId  =Application::instance().marketData()->source(sourceName)->coinIdFromName(coinName);
    const md::Source::Coin  coin    =Application::instance().marketData()->source(sourceName)->coin(coinId);
    source.selectedCoinID   =coinId;
    if(!coin.name.isEmpty())
    {
        ui->assetName->setText(coin.name);
        ui->assetSymbol->setText(coin.symbol);
    }
    refreshSourceData(source);
    TM_TRACE(TAG,"New selected coin id: "+coinId);
}

//
void                            MarketDataEditAssetDialog::onAssetTypeChanged(int)
{
    ui->tabWidget->setTabEnabled(0,ui->assetType->currentText()!="fiat");
    ui->tabWidget->setTabEnabled(1,ui->assetType->currentText()!="fiat");
}

//
void                            MarketDataEditAssetDialog::onAssetExchangeChanged(int)
{
    const QString               exchange=sender()->property(PROPERTY_EXCHANGE_ID).toString();
    QComboBox*                  combo   =dynamic_cast<QComboBox*>(sender());
    const QString               symbol  =combo->currentData().toString();
    TM_TRACE(TAG,QString("%1: %2").arg(exchange).arg(symbol));
    _exchanges[exchange]=symbol;
}
