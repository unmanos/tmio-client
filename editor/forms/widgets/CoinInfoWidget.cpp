#include "CoinInfoWidget.h"
#include "Application.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTimer>
#include <QTabWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QGridLayout>
#include <QModelIndex>
#include <QAction>
#include <QCheckBox>

#include <md/Asset.h>
#include <md/Exchange.h>
#include "libs/ui/sort/GenericSortFilterProxyModel.h"
#include "libs/ui/asset/AssetPairsWidget.h"

//
#define TAG                             "CoinInfoWidget"
#define ICON_INFO                       ":/icons/common/info.png"
#define ICON_PAIRS                      ":/icons/common/chain-link.png"
#define ICON_SOCIAL                     ":/icons/common/share-this.png"
#define ICON_CHAIN                      ":/icons/mining.png"

int CoinInfoWidget::_currentPage        =0;

//
CoinInfoWidget::CoinInfoWidget(QString assetID,QWidget *parent) : QWidget(parent)
{
    CoinMarketCap*                    cmc       =Application::instance().cmc();
    md::MarketData*                   md        =Application::instance().marketData();
    md::Asset*                        asset     =md->asset(assetID);
    QVBoxLayout*                      winLayout =new QVBoxLayout();
    _assetID        =assetID;
    setLayout(winLayout);
    // title
    {
        QHBoxLayout*    titleLayout=new QHBoxLayout(this);
        titleLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QLabel*         labelIcon=new QLabel();
        labelIcon->setPixmap(Application::instance().cmc()->assetIcon(_assetID).pixmap(32,32));
        //labelIcon->setAlignment(Qt::TopLeftCorner);

        QLabel*         labelName=new QLabel(asset->name());
        QFont           font=labelName->font();
        font.setPointSize(16);
        labelName->setFont(font);
        labelName->setAlignment(Qt::AlignLeft);
        //labelName->setSizePolicy(QSizePolicy::Minimum);

        titleLayout->addWidget(labelIcon);
        titleLayout->addWidget(labelName);
        titleLayout->addStretch(1);

        if(cmc->assetRank(_assetID))
        {
            QLabel*         rankIcon=new QLabel();
            rankIcon->setPixmap(QIcon(":/icons/common/rank.png").pixmap(20,20));
            QLabel*         rankValue=new QLabel(QString::number(cmc->assetRank(_assetID)));
            QFont           font=labelName->font();
            font.setPointSize(12);
            //font.set
            rankValue->setFont(font);
            //rankValue->setText
            titleLayout->addWidget(rankIcon);
            titleLayout->addWidget(rankValue);
        }
        winLayout->addLayout(titleLayout);
    }
    // tab
    _tabWidget                                  =new QTabWidget();
    // size
    setMinimumHeight(256);
    setMaximumHeight(384);
    // add pages
    buildTabInfo();
    buildTabPairs();
    buildTabChain();
    //buildTabSocial();
    // add tab
    winLayout->addWidget(_tabWidget);
    _tabWidget->setCurrentIndex(_currentPage);
    // update code
    connect(_tabWidget,SIGNAL(currentChanged(int)),this,SLOT(onCurrentTabChanged(int)));
    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onRefreshData()));
    connect(Application::instance().marketData(),&md::MarketData::assetModified,this,&CoinInfoWidget::onAssetModified);
    QTimer*          refreshTimer=new QTimer(this);
    connect(refreshTimer,SIGNAL(timeout()),this,SLOT(onRefreshData()));
    refreshTimer->start(1000);
    refreshTabInfo();
    refreshTabChain();
}

//
void        CoinInfoWidget::buildTabInfo()
{
    md::MarketData*     md      =Application::instance().marketData();
    md::Asset*          asset   =md->asset(_assetID);
    QWidget*            pageInfo=new QWidget();
    _tabWidget->addTab(pageInfo,QIcon(ICON_INFO),"Info");
    //QGridLayout*    layout=new QGridLayout();
    //pageInfo->setLayout(layout);
    // infos
    {
        _infoLayout =new QFormLayout(pageInfo);
        _infoLayout->setSizeConstraint(QLayout::SetMaximumSize);
        QString         symbol      =asset->symbol();
        if(asset->sign().length())
        {
            symbol=QString("%1, %2").arg(asset->symbol()).arg(asset->sign());
        }
        _watchCheckbox=new QCheckBox();
        _watchCheckbox->setCheckState(md->watchAsset(_assetID)?Qt::Checked:Qt::Unchecked);
        //md->setFavoriteAsset(asset,favorite);

        _infoLayout->addRow("Watch",_watchCheckbox);
        _infoLayout->addRow("Type",new QLabel(QString("%1").arg(asset->type())));
        _infoLayout->addRow(asset->sign().length()?"Symbols":"Symbol",new QLabel(symbol));
        _infoLayout->addRow("Rank",_labelRank=new QLabel());
        _infoLayout->addRow("Market cap",_labelMarketCap=new QLabel("?"));
        _infoLayout->addRow("Price",_labelPrice=new QLabel("?"));

        _infoLayout->addRow(nullptr,_labelPriceConvert=new QLabel(""));
        QFont           convertFont=_labelPriceConvert->font();
        convertFont.setPointSize(convertFont.pointSize()*0.75);
        _labelPriceConvert->setFont(convertFont);
        //_infoLayout->labelForField(_labelPriceConvert)->setFont(convertFont);
        _infoLayout->addRow("Circulating supply",_labelCirculatingSupply=new QLabel(QString("?")));
        connect(_watchCheckbox,SIGNAL(toggled(bool)),this,SLOT(onChangeWatch(bool)));
    }
    pageInfo->setLayout(_infoLayout);
}

//
void        CoinInfoWidget::buildTabPairs()
{
    AssetPairsWidget*           pairs=new AssetPairsWidget(_assetID);
    _tabWidget->addTab(pairs,QIcon(ICON_PAIRS),"Pairs");
    connect(pairs,SIGNAL(openPair(const QString&,const QString&)),this,SLOT(onOpenPair(const QString&,const QString&)));
}

//
void        CoinInfoWidget::buildTabChain()
{
    md::MarketData*     md      =Application::instance().marketData();
    md::Asset*          asset   =md->asset(_assetID);
    if(!asset->data()["chain"].toObject().isEmpty())
    {
        QWidget*            pageInfo=new QWidget();
        _tabWidget->addTab(pageInfo,QIcon(ICON_CHAIN),"Chain");
        //QGridLayout*    layout=new QGridLayout();
        //pageInfo->setLayout(layout);
        // infos
        {
            _chainLayout =new QFormLayout(pageInfo);
            _chainLayout->setSizeConstraint(QLayout::SetMaximumSize);
        }
        pageInfo->setLayout(_chainLayout);
    }
}

//
void        CoinInfoWidget::buildTabSocial()
{
    QWidget*                          pageSocial =new QWidget();
    _tabWidget->addTab(pageSocial,QIcon(ICON_SOCIAL),"Social");
    QGridLayout*    layout=new QGridLayout();
    pageSocial->setLayout(layout);
}

//
void        CoinInfoWidget::refreshTabInfo()
{
    CoinMarketCap*                    cmc               =Application::instance().cmc();
    md::MarketData*                   marketData        =Application::instance().marketData();
    md::Asset*                        asset             =marketData->asset(_assetID);
    double                            circulatingSupply =cmc->circulatingSupply(asset->id());
    //bool                              CoinMarketCap::assetPrice(const QString& quoteAssetID,const QString& assetID,double& amount,int recurse)
    double                            assetPrice         =1;
    QVector<QString>                  priceRoute;
    //_infoLayout->addRow(nullptr,new QLabel("FUCK YOU QT!"));
    if(cmc->assetPrice(cmc->quoteAssetID(),asset->id(),assetPrice,&priceRoute))
    {
        QString routeText=asset->symbol();
        foreach(QString routeAssetID,priceRoute)
        {
            QString s=marketData->asset(routeAssetID)->symbol();
            routeText+=" > "+s;
        }
        routeText+=" > "+marketData->asset(cmc->quoteAssetID())->symbol();
        _labelPriceConvert->setText(routeText);
        _labelPrice->setText(cmc->formatFiatCurrency(assetPrice,cmc->quoteAssetID()));
        if(circulatingSupply>0)
        {
            _labelMarketCap->setText(cmc->formatFiatCurrency(circulatingSupply*assetPrice,cmc->quoteAssetID()));
        }
    }
    else
    {
        _labelPrice->setText("?");
    }
    if(circulatingSupply>0)
    {
        _labelCirculatingSupply->setText(cmc->formatSupply(circulatingSupply));
    }
    //layout->addStretch(1);
}

static QString  _humanElapsedTime(qint64 elapsed)
{
    QString     text;
    bool        includeSeconds=true;
    if(elapsed>=(24*3600))
    {
        int days=elapsed/(24*3600);
        text+=QString("%1 day%2").arg(days).arg((days>1)?"s":"");
        elapsed-=days*24*3600;
        includeSeconds=false;
    }
    if(elapsed>=3600)
    {
        int hours=elapsed/3600;
        if(text.length()) text+=" ";
        text+=QString("%1 hour%2").arg(hours).arg((hours>1)?"s":"");
        elapsed-=hours*3600;
        includeSeconds=false;
    }
    if(elapsed>=60)
    {
        int minutes=elapsed/60;
        if(text.length()) text+=" ";
        text+=QString("%1 minute%2").arg(minutes).arg((minutes>1)?"s":"");
        elapsed-=minutes*60;
    }
    if( (elapsed)&&(includeSeconds) )
    {
        if(text.length()) text+=" ";
        text+=QString("%1 second%2").arg(elapsed).arg((elapsed>1)?"s":"");
    }
    return(text);
}

//
void        CoinInfoWidget::refreshTabChain()
{
    if(!_chainLayout)
        return;
    md::MarketData* marketData  =Application::instance().marketData();
    md::Asset*      asset        =marketData->asset(_assetID);
    while(_chainLayout->rowCount())
        _chainLayout->removeRow(_chainLayout->rowCount()-1);

    QJsonObject     chain       =asset->data()["chain"].toObject();
    if(chain.contains("pow"))
    {
        _chainLayout->addRow(new QLabel("PoW algorithm"),new QLabel(QString("%1").arg(chain["pow"].toString())));
    }
    if(chain.contains("height"))
    {
        _chainLayout->addRow(new QLabel("Height"),new QLabel(QString("%1").arg(chain["height"].toInt())));
    }
    if(chain.contains("time"))
    {
        QDateTime   time    =chain["time"].toVariant().toDateTime();
        qint64      elapsed =qMax(static_cast<qint64>(0),time.secsTo(QDateTime::currentDateTime()));
        _chainLayout->addRow(new QLabel("Block found"),new QLabel(elapsed?(_humanElapsedTime(elapsed)+" ago"):"now"));
    }
    if(chain.contains("reward"))
    {
        _chainLayout->addRow(new QLabel("Reward"),new QLabel(QString("%1").arg(chain["reward"].toDouble())));
    }
    if(chain.contains("halvingTime"))
    {
        QDateTime   time    =chain["halvingTime"].toVariant().toDateTime();
        qint64      elapsed =qMax(static_cast<qint64>(0),QDateTime::currentDateTime().secsTo(time));
        _chainLayout->addRow(new QLabel("Next halving"),new QLabel(time.toLocalTime().toString()));
        _chainLayout->addRow(new QLabel(""),new QLabel(_humanElapsedTime(elapsed)));
    }
    if(chain.contains("retargetTime"))
    {
        QDateTime   time    =chain["retargetTime"].toVariant().toDateTime();
        qint64      elapsed =qMax(static_cast<qint64>(0),QDateTime::currentDateTime().secsTo(time));
        _chainLayout->addRow(new QLabel("Next retarget"),new QLabel(_humanElapsedTime(elapsed)));
    }


    //_chainLayout->remo
}

//
void        CoinInfoWidget::onRefreshData()
{
    refreshTabInfo();
    refreshTabChain();
}

//
void        CoinInfoWidget::onCurrentTabChanged(int index)
{
    _currentPage=index;
}

//
void        CoinInfoWidget::onChangeWatch(bool)
{
    md::MarketData*         md          =Application::instance().marketData();
    if(!_assetID.isEmpty())
    {
        TM_TRACE(TAG,"Watch changed");
        md->setWatchAsset(_assetID,_watchCheckbox->checkState()==Qt::Checked);
    }
}

//
void        CoinInfoWidget::onOpenPair(const QString& exchange,const QString& pair)
{

    QAction                 action("Open ticker");
    //TM_TRACE(TAG,QString("Open ticker index %1 (%2), exchange %3, asset %4").arg(index.row()).arg(ticker).arg(exchange).arg(_assetID));
    action.setProperty(PROP_TOOL_NAME,"exchange/openticker");
    //action.setProperty(PROP_TOOL_TOOL_DISPLAY_NAME,ticker);
    action.setProperty(PROP_TOOL_EXCHANGE_NAME,exchange);
    action.setProperty(PROP_TOOL_TICKER_NAME,pair);
    action.setProperty(PROP_TOOL_ASSET_ID,_assetID);
    emit sigActionTriggered(&action);
}

//
void        CoinInfoWidget::onAssetModified(const QString& id)
{
    if(_assetID==id)
    {
        onRefreshData();
    }
}
