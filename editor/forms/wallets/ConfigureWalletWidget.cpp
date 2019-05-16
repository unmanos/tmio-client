#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include "ConfigureWalletWidget.h"
#include "Application.h"
#include <md/MarketData.h>
#include <md/Exchange.h>
#include <md/Asset.h>

//
#define TAG                 "ConfigureWalletWidget"
//#define WIDGET_CATEGORY_ID  (Qt::UserRole+2)
#define PROP_ARGUMENT_KEY   "tmio.argumentkey"

//
ConfigureWalletWidget::ConfigureWalletWidget(int walletID,QJsonObject walletConfig) : QWidget(nullptr)
{
    TM_TRACE(TAG,"LOAD: "+QJsonDocument(walletConfig).toJson());
    _id         =walletID;
    _config     =walletConfig;
    _formLayout =new QFormLayout();
    setLayout(_formLayout);
    // set exchange parameters
    setExchangeArgument("binance","apiKey","API Key");
    setExchangeArgument("binance","apiSecret","API Secret");
    setExchangeArgument("bitstamp","customerID","Customer ID");
    setExchangeArgument("bitstamp","apiKey","API Key");
    setExchangeArgument("bitstamp","apiSecret","API Secret");
    setExchangeArgument("kraken","apiKey","API Key");
    setExchangeArgument("kraken","apiSecret","API Secret");
    setExchangeArgument("coinbase","apiKey","API Key");
    setExchangeArgument("coinbase","apiSecret","API Secret");
    // set mining parameters
    setMiningPoolArgument("nicehash","apiID","API ID");
    setMiningPoolArgument("nicehash","apiKey","API Key");
    setMiningPoolArgument("nicehash","address","Address");
    // set mining parameters
    setMiningPoolArgument("ethermine","address","Address");
    // set private parameters
    setPrivateArgument("bitcoin","address","Address");
    setPrivateArgument("litecoin","address","Address");
    setPrivateArgument("ethereum","address","Address");
    // set json rpc parameters
    setJsonRpcArgument("bitcoin","address","Address");
    setJsonRpcArgument("bitcoin","username","Username");
    setJsonRpcArgument("bitcoin","password","Password");
    setJsonRpcArgument("litecoin","address","Address");
    setJsonRpcArgument("litecoin","username","Username");
    setJsonRpcArgument("litecoin","password","Password");
    for(int i=0;i<_args.size();i++)
    {
        const QStringList   key=(_args[i].path+"/"+_args[i].name).split("/");
        QJsonValue          v=_config;
        foreach(QString k,key)
        {
            v=v.toObject()[k];
        }
        _args[i].value=v.toString();
        //walletConfig[]

    }

    // add name
    _nameLineEdit=new QLineEdit();
    _formLayout->addRow("Name",_nameLineEdit);
    _nameLineEdit->setText(walletConfig["name"].toString());
    if(walletConfig["name"].toString().length()==0)
    {
        _isDefaultName=true;
    }
    connect(_nameLineEdit,&QLineEdit::textChanged,this,&ConfigureWalletWidget::onNameChanged);
    // add type
    _typeCombo   =new QComboBox();
    _formLayout->addRow("Type",_typeCombo);
    //
    _formLayout->addItem(new QSpacerItem(0,10));
    //
    _typeCombo->addItem(QIcon(":/icons/assets.png"),tr("Exchange"),"exchange");
    _typeCombo->addItem(QIcon(":/icons/mining.png"),tr("Mining pool"),"mining");
    _typeCombo->addItem(QIcon(":/icons/sync.png"),tr("Private sync"),"private");
    _typeCombo->addItem(QIcon(":/icons/json.png"),tr("JSON RPC"),"jsonrpc");
    if(walletConfig["exchange"].isObject())
        _typeCombo->setCurrentIndex(0);
    if(walletConfig["mining"].isObject())
        _typeCombo->setCurrentIndex(1);
    if(walletConfig["private"].isObject())
        _typeCombo->setCurrentIndex(2);

    connect(_typeCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onTypeChanged(int)));
    onTypeChanged(0);


    //onTypeChanged(0);
    updateChanges();
}

//
QJsonObject         ConfigureWalletWidget::config()
{
    return(_config);
}

//
void                ConfigureWalletWidget::setExchangeArgument(const QString& exchange,const QString& name,const QString& text)
{
    Argument        arg;
    arg.path          ="exchange/"+exchange;
    arg.name          =name;
    arg.text          =text;
    _args.append(arg);
}

//
void                ConfigureWalletWidget::setMiningPoolArgument(const QString& pool,const QString& name,const QString& text)
{
    Argument        arg;
    arg.path          ="mining/"+pool;
    arg.name          =name;
    arg.text          =text;
    _args.append(arg);
}

//
void                ConfigureWalletWidget::setPrivateArgument(const QString& asset,const QString& name,const QString& text)
{
    Argument        arg;
    arg.path          ="private/"+asset;
    arg.name          =name;
    arg.text          =text;
    _args.append(arg);
}

//
void                ConfigureWalletWidget::setJsonRpcArgument(const QString& asset,const QString& name,const QString& text)
{
    Argument        arg;
    arg.path          ="jsonrpc/"+asset;
    arg.name          =name;
    arg.text          =text;
    _args.append(arg);
}

//
void                ConfigureWalletWidget::setArgumentValue(const QString& key,const QString& text)
{
    for(int i=0;i<_args.size();i++)
    {
        if( (_args[i].path+"/"+_args[i].name)==key)
        {
            _args[i].value=text;
            updateChanges();
        }
    }
}

//
void                ConfigureWalletWidget::updateChanges()
{
    QJsonObject     config;
    const QString   walletType  =_typeCombo->currentData().toString();
    const QString   location    =_locationCombo->currentData().toString();
    config.insert("id",_id);
    config.insert("name",_nameLineEdit->text());
    QJsonObject     settings;
    QJsonObject     arguments;
    for(int i=0;i<_args.size();i++)
    {
        if(_args[i].path==(walletType+"/"+location))
        {
            arguments.insert(_args[i].name,_args[i].value);
        }
    }
    settings.insert(location,arguments);
    config.insert(walletType,settings);
    _config         =config;
    TM_TRACE(TAG,"SAVE: "+QJsonDocument(config).toJson());
    emit walletChanged(_id,config);
}

//
void                ConfigureWalletWidget::createArgsControls(const QString& startPath)
{
    // remove previous
    foreach(QWidget* widget,_editorWidgets)
    {
        if(_locationCombo!=widget)
            _formLayout->removeRow(widget);
    }
    _editorWidgets.clear();
    _editorWidgets.append(_locationCombo);
    // set new
    foreach(Argument arg,_args)
    {
        if(arg.path.startsWith(startPath))
        {
            QLineEdit*      lineEdit=new QLineEdit();
            lineEdit->setProperty(PROP_ARGUMENT_KEY,arg.path+"/"+arg.name);
            _formLayout->addRow(arg.text,lineEdit);
            _editorWidgets.append(lineEdit);
            lineEdit->setText(arg.value);
            connect(lineEdit,&QLineEdit::textChanged,this,&ConfigureWalletWidget::onArgumentChanged);
        }
    }
}

//
void                ConfigureWalletWidget::onTypeChanged(int /*index*/)
{
    CoinMarketCap*      cmc =Application::instance().cmc();
    md::MarketData*     md  =Application::instance().marketData();
    // clear exchange
    foreach(QWidget* widget,_editorWidgets)
    {
        _formLayout->removeRow(widget);
    }
    _editorWidgets.clear();
    _locationCombo=nullptr;
    // build new exchange widgets
    if(_typeCombo->currentData().toString()=="exchange")
    {
        _locationCombo   =new QComboBox();
        foreach(const QString exchangeID,Application::instance().marketData()->exchanges())
        {
            bool supported=false;
            foreach(Argument arg,_args)
            {
                if(arg.path.startsWith("exchange/"+exchangeID))
                {
                    supported=true;
                }
            }
            if(supported)
            {
                md::Exchange*   exchange=Application::instance().marketData()->exchange(exchangeID);
                _locationCombo->addItem(cmc->exchangeIcon(exchangeID),exchange->name(),exchange->id());
                if(!_config["exchange"].toObject()[exchangeID].isNull())
                    _locationCombo->setCurrentText(exchange->name());
            }
        }
        _formLayout->addRow("Exchange",_locationCombo);
        _editorWidgets.append(_locationCombo);
        connect(_locationCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onExchangeChanged(int)));
        onExchangeChanged(0);
    }
    // build new mining widgets
    if(_typeCombo->currentData().toString()=="mining")
    {
        QStringList     allowedPools;
        allowedPools.append("nicehash");
        allowedPools.append("ethermine");
        _locationCombo    =new QComboBox();
        foreach(QString poolID,allowedPools)
        {
            //md->asset(assetID)->
            _locationCombo->addItem(cmc->miningPoolIcon(poolID),cmc->miningPoolName(poolID),poolID);
            if(!_config["mining"].toObject()[poolID].isNull())
                _locationCombo->setCurrentText(cmc->miningPoolName(poolID));
        }
        //_locationCombo->setCurrentText(md->asset(assetID)->name());

        _formLayout->addRow("Mining pool",_locationCombo);
        _editorWidgets.append(_locationCombo);
        connect(_locationCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onMiningPoolChanged(int)));
        onMiningPoolChanged(0);
    }
    // build new private widgets
    if(_typeCombo->currentData().toString()=="private")
    {
        QStringList     allowedAssets;
        allowedAssets.append("bitcoin");
        allowedAssets.append("litecoin");
        allowedAssets.append("ethereum");
        _locationCombo   =new QComboBox();
        foreach(QString assetID,allowedAssets)
        {
            //md->asset(assetID)->
            _locationCombo->addItem(cmc->assetIcon(assetID),md->asset(assetID)->name(),assetID);
            if(!_config["private"].toObject()[assetID].isNull())
                _locationCombo->setCurrentText(md->asset(assetID)->name());
        }
        _formLayout->addRow("Asset",_locationCombo);
        _editorWidgets.append(_locationCombo);
        connect(_locationCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onPrivateCoinChanged(int)));
        onPrivateCoinChanged(0);
    }
    // build new jsonrpc widgets
    if(_typeCombo->currentData().toString()=="jsonrpc")
    {
        QStringList     allowedAssets;
        allowedAssets.append("bitcoin");
        allowedAssets.append("litecoin");
        _locationCombo   =new QComboBox();
        foreach(QString assetID,allowedAssets)
        {
            //md->asset(assetID)->
            _locationCombo->addItem(cmc->assetIcon(assetID),md->asset(assetID)->name(),assetID);
            if(!_config["jsonrpc"].toObject()[assetID].isNull())
                _locationCombo->setCurrentText(md->asset(assetID)->name());
        }
        _formLayout->addRow("Asset",_locationCombo);
        _editorWidgets.append(_locationCombo);
        connect(_locationCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onJsonRpcCoinChanged(int)));
        onJsonRpcCoinChanged(0);
    }
}

//
void                ConfigureWalletWidget::onNameChanged(const QString& /*text*/)
{
    TM_TRACE(TAG,"onNameChanged");
    _isDefaultName=false;
    updateChanges();
}

//
void                ConfigureWalletWidget::onExchangeChanged(int /*index*/)
{
    TM_TRACE(TAG,"onExchangeChanged");
    const QString   exchangeID=_locationCombo->currentData().toString();
    if(_isDefaultName)
    {
        _nameLineEdit->setText(QString(tr("My %1 wallet")).arg(Application::instance().marketData()->exchange(exchangeID)->name()));
        _isDefaultName=true;
    }
    createArgsControls("exchange/"+exchangeID);
    updateChanges();
}

//
void                ConfigureWalletWidget::onMiningPoolChanged(int /*index*/)
{
    TM_TRACE(TAG,"onMiningPoolChanged");
    const QString   miningPoolID=_locationCombo->currentData().toString();
    if(_isDefaultName)
    {
        _nameLineEdit->setText(QString(tr("My %1 mine")).arg(Application::instance().cmc()->miningPoolName(miningPoolID)));
        _isDefaultName=true;
    }
    createArgsControls("mining/"+miningPoolID);
    updateChanges();
}

//
void                ConfigureWalletWidget::onPrivateCoinChanged(int /*index*/)
{
    TM_TRACE(TAG,"onPrivateCoinChanged");
    md::MarketData*     md  =Application::instance().marketData();
    const QString   assetID=_locationCombo->currentData().toString();
    if(_isDefaultName)
    {
        _nameLineEdit->setText(QString(tr("My %1 address")).arg(md->asset(assetID)->name()));
        _isDefaultName=true;
    }
    createArgsControls("private/"+assetID);
    updateChanges();
}

//
void                ConfigureWalletWidget::onJsonRpcCoinChanged(int /*index*/)
{
    TM_TRACE(TAG,"onJsonRpcCoinChanged");
    md::MarketData*     md      =Application::instance().marketData();
    const QString       assetID =_locationCombo->currentData().toString();
    if(_isDefaultName)
    {
        _nameLineEdit->setText(QString(tr("My %1 desktop wallet")).arg(md->asset(assetID)->name()));
        _isDefaultName=true;
    }
    createArgsControls("jsonrpc/"+assetID);
    updateChanges();
}

//
void                ConfigureWalletWidget::onArgumentChanged(const QString& text)
{
    TM_TRACE(TAG,"onArgumentChanged");
    QLineEdit*      lineEdit=dynamic_cast<QLineEdit*>(sender());
    const QString   key     =lineEdit->property(PROP_ARGUMENT_KEY).toString();
    // update argument
    setArgumentValue(key,text);
}
