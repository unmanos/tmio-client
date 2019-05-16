#include "Application.h"
#include "ConfigureWalletsDialog.h"
#include "ConfigureWalletWidget.h"
#include "ui_ConfigureWalletsDialog.h"

//
#define TAG             "ConfigureWalletsDialog"
#define WALLET_ITEM_ID  (Qt::UserRole+2)

//
ConfigureWalletsDialog::ConfigureWalletsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureWalletsDialog)
{
    ui->setupUi(this);
    ui->walletsList->setIconSize(QSize(16,16));
    const QByteArray    savedWallets=Application::instance().settings().encryptedValue(SETTINGS_WALLETS_CONFIGS);
    QJsonDocument       savedDoc=QJsonDocument::fromJson(savedWallets);
    if(savedDoc.isArray())
    {
        for(int i=0;i<savedDoc.array().count();i++)
        {
            QJsonObject         jso=savedDoc.array().at(i).toObject();
            int                 id=jso["id"].toInt();
            _wallets[id]        =jso;
        }
    }
    refreshWalletList(0);
    connect(ui->addWalletButton,&QAbstractButton::pressed,this,&ConfigureWalletsDialog::onAddWallet);
    connect(ui->removeWalletButton,&QAbstractButton::pressed,this,&ConfigureWalletsDialog::onRemoveWallet);
    connect(ui->walletsList,SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,SLOT(onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));
    ui->removeWalletButton->setEnabled(false);
}

//
ConfigureWalletsDialog::~ConfigureWalletsDialog()
{
    delete ui;
}

//
QListWidgetItem*            ConfigureWalletsDialog::itemFromWalletId(int id)
{
    for(int i=0;i<ui->walletsList->count();i++)
    {
        if(ui->walletsList->item(i)->data(WALLET_ITEM_ID).toInt()==id)
            return(ui->walletsList->item(i));
    }
    return(nullptr);
}

//
void                        ConfigureWalletsDialog::refreshWalletList(int selected)
{
    // add&update list
    foreach(int id,_wallets.keys())
    {
        QJsonObject         wallet  =_wallets[id];
        QListWidgetItem*    item    =itemFromWalletId(id);
        if(!item)
        {
            item=new QListWidgetItem();
            item->setData(WALLET_ITEM_ID,id);
            ui->walletsList->addItem(item);
        }
        item->setText(wallet["name"].toString());
        item->setIcon(QIcon());
        foreach(const QString exchangeID,Application::instance().marketData()->exchanges())
        {
            if(wallet["exchange"].toObject()[exchangeID].isObject())
            {
                item->setIcon(Application::instance().cmc()->exchangeIcon(exchangeID));
            }
        }
        if(wallet["mining"].isObject())
        {
            const QString       miningPoolID=wallet["mining"].toObject().keys().first();
            item->setIcon(Application::instance().cmc()->miningPoolIcon(miningPoolID));
        }
        if(wallet["private"].isObject())
        {
            const QString       assetID=wallet["private"].toObject().keys().first();
            item->setIcon(Application::instance().cmc()->assetIcon(assetID));
        }
        if(wallet["jsonrpc"].isObject())
        {
            const QString       assetID=wallet["jsonrpc"].toObject().keys().first();
            item->setIcon(Application::instance().cmc()->assetIcon(assetID));
        }

    }
    ui->walletsList->setCurrentItem(itemFromWalletId(selected));
}

//
void                        ConfigureWalletsDialog::accept()
{
    TM_TRACE(TAG,"accept");
    // seralize all wallets
    // TODO: check for errors (empty keys, ...)
    QJsonArray              jsonArray;
    foreach(int id,_wallets.keys())
    {
        QJsonObject         jsonWallet;
        jsonArray.append(_wallets[id]);
    }
    // save
    Application::instance().settings().setEncryptedValue(SETTINGS_WALLETS_CONFIGS,QJsonDocument(jsonArray).toJson(QJsonDocument::Compact));
    QDialog::accept();
}

//
void                        ConfigureWalletsDialog::onAddWallet()
{
    TM_TRACE(TAG,"onAddWallet");
    QJsonObject wallet;
    int         id          =(_wallets.count()?_wallets.lastKey():0)+1;
    _wallets.insert(id,wallet);
    refreshWalletList(id);
}

//
void                        ConfigureWalletsDialog::onRemoveWallet()
{
    TM_TRACE(TAG,"onRemoveWallet");
    QListWidgetItem*        currentItem=ui->walletsList->currentItem();
    if(currentItem)
    {
        int id=currentItem->data(WALLET_ITEM_ID).toInt();
        _wallets.remove(id);
        delete(currentItem);
        refreshWalletList(0);
    }
    ui->removeWalletButton->setEnabled(false);
}

void                        ConfigureWalletsDialog::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    TM_TRACE(TAG,"onCurrentItemChanged");
    if(_currentWalletWidget)
    {
        delete(_currentWalletWidget);
        _currentWalletWidget=nullptr;
    }
    if(current)
    {
        int id=current->data(WALLET_ITEM_ID).toInt();
        _currentWalletWidget=new ConfigureWalletWidget(id,_wallets[id]);
        ui->configContainer->layout()->addWidget(_currentWalletWidget);
        connect(_currentWalletWidget,&ConfigureWalletWidget::walletChanged,this,&ConfigureWalletsDialog::onWalletConfigChanged);
        onWalletConfigChanged(id,_currentWalletWidget->config());
        ui->removeWalletButton->setEnabled(true);
    }
    //updateWalletUI();
}

//
void                        ConfigureWalletsDialog::onWalletConfigChanged(int id,QJsonObject config)
{
    _wallets[id]=config;
    refreshWalletList(id);
}
