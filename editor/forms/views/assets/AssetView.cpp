#include "AssetView.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QGroupBox>
#include <QTabWidget>
#include <QGridLayout>
#include <md/Asset.h>

#include "Application.h"
#include "libs/ui/asset/AssetPairsWidget.h"
#include "libs/ui/community/CommunityChannel.h"

#define TAG                     "AssetView"

using namespace tmio;

//
AssetView::AssetView(QString assetID,QWidget *parent) : QWidget(parent), _assetID(assetID)
{
    setLayout(new QVBoxLayout());
    md::Asset*          asset=Application::instance().marketData()->asset(_assetID);
    // setup title frame
    {
        QFrame*             titleFrame=new QFrame();
        QHBoxLayout*        titleLayout=new QHBoxLayout();
        titleFrame->setLayout(titleLayout);
        titleLayout->setMargin(0);
        titleLayout->addSpacing(8);
        if(asset)
        {
            connect(asset,&md::Asset::priceChanged,this,&AssetView::onPriceChanged);
            connect(Application::instance().cmc(),&CoinMarketCap::quoteAssetChanged,this,&AssetView::onPriceChanged);
            // add coin icon
            {
                QLabel*             coinIcon=new QLabel();
                coinIcon->setPixmap(asset->icon().pixmap(30,30));
                titleLayout->addWidget(coinIcon);
                //topFormLayout->addStretch(1);
            }

            // add pair info
            {
                QString rt;
                rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                                "margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;\">";
                rt+=QString("<span style=\"font-size:11pt;\"><b>%1</b></span>").arg(asset->name());
                //rt+=QString("<span style=\"font-size:9pt;\"> / %1</span>").arg(_quoteTicker);
                rt+="<br>";
                rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg(asset->symbol());
                rt+="</div>";

                QLabel *label = new QLabel();
                label->setText(rt);
                titleLayout->addWidget(label);
                titleLayout->addStretch(1);
            }
        }
        // add last price
        {
            _labelLastPrice = new QLabel();
            //_labelLastPriceFiat = new QLabel();
            titleLayout->addWidget(_labelLastPrice);
            titleLayout->addSpacing(20);
            //topFormLayout->addWidget(_labelLastPriceFiat);
            titleLayout->addStretch(1);
        }
        // add last price
        {
            _labelMarketCap = new QLabel();
            //_labelLastPriceFiat = new QLabel();
            titleLayout->addWidget(_labelMarketCap);
            titleLayout->addSpacing(20);
            //topFormLayout->addWidget(_labelLastPriceFiat);
            titleLayout->addStretch(1);
        }
        layout()->addWidget(titleFrame);
    }
    // community layout
    {
        QTabWidget*         communityFrame=new QTabWidget();
        QHBoxLayout*        communityLayout=new QHBoxLayout();
        communityFrame->setLayout(communityLayout);
        communityLayout->setMargin(0);
        //communityLayout->addSpacing(8);
        communityFrame->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

        communityFrame->addTab(new CommunityChannel(),QIcon(":/icons/common/chat.png"),"Chat");
        communityFrame->addTab(new QLabel(),"News");
        communityFrame->addTab(new QLabel(),"Twitter");
        communityFrame->addTab(new QLabel(),"Alarms");
        communityFrame->addTab(new QLabel(),"Price targets");
        //communityFrame->addTab(new QLabel(),"Alarms");

        layout()->addWidget(communityFrame);
    }
    onPriceChanged();
}

//
void                AssetView::onPriceChanged()
{
    //TM_TRACE(TAG,"Price update received");
    CoinMarketCap*      cmc         =Application::instance().cmc();
    QString             priceStr="?";
    QString             marketCapStr="?";
    double              assetPrice=1;
    if(cmc->assetPrice(cmc->quoteAssetID(),_assetID,assetPrice,nullptr))
    {
        priceStr=cmc->formatFiatCurrency(assetPrice,cmc->quoteAssetID());
        const double    circulatingSupply =cmc->circulatingSupply(_assetID);
        if(circulatingSupply>0)
        {
            marketCapStr=cmc->formatFiatCurrency(assetPrice*circulatingSupply,cmc->quoteAssetID());
        }
    }
    {
        QString rt;
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
        rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("Last price");
        rt+="<br>";
        rt+=QString("<span style=\"font-size:11pt;\"><b>%1 </b></span>").arg(priceStr);
        rt+="</div>";
        _labelLastPrice->setText(rt);
    }
    {
        QString rt;
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
        rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("Market cap");
        rt+="<br>";
        rt+=QString("<span style=\"font-size:11pt;\"><b>%1 </b></span>").arg(marketCapStr);
        rt+="</div>";
        _labelMarketCap->setText(rt);
    }


}
