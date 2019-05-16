#include <QLabel>
#include <QTabWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QTabBar>

#include "CommunityWidget.h"
#include "CommunityPageAsset.h"
#include "Application.h"
#include "MainWindow.h"

#include <md/Asset.h>

//
#define TAG             "SocialWidget"
#define PROP_ASSET_ID   "assetID"

//
CommunityWidget::CommunityWidget(QWidget *parent) : QWidget(parent)
{
    // make sure we have a subscriber
    Application::instance().communitySubscriber();
    //QLabel*     label=new QLabel("sdsdsdsd",this);
    QGridLayout*    layout=new QGridLayout();
    setLayout(layout);

    _tabWidget=new QTabWidget();
    /*tabWidget->addTab(new QWidget(),"Bitcoin");
    tabWidget->addTab(new QWidget(),"Ethereum");
    tabWidget->addTab(new QWidget(),"Stellar");*/
    layout->addWidget(_tabWidget);
    _tabWidget->setTabsClosable(true);

    _tabWidget->setStyleSheet(
        "QCheckBox::indicator:unchecked {image: url(:/icons/common/checkbox-star-unchecked.png); width: 16px; height: 16px; }"
        "QCheckBox::indicator:checked {image: url(:/icons/common/checkbox-star-checked.png); width: 16px; height: 16px; }"
      );

    connect(_tabWidget,&QTabWidget::tabCloseRequested,this,&CommunityWidget::onTabCloseRequested);
    //tabCloseRequested(int index)
    openAsset("bitcoin");

}

//
void                    CommunityWidget::openAsset(const QString& assetID)
{
    TM_TRACE(TAG,"openAsset: "+assetID);
    md::MarketData*     md      =Application::instance().marketData();
    CoinMarketCap*      cmc     =Application::instance().cmc();
    md::Asset*          asset   =md->asset(assetID);
    //_tabWidget->
    if(asset)
    {
        removeUnpinnedTabs();
        if(!_pages.contains(assetID))
        {
            QWidget*        page        =new CommunityPageAsset(assetID);
            page->setProperty(PROP_ASSET_ID,assetID);
            int             pageIndex   =_tabWidget->addTab(page,cmc->assetIcon(assetID),asset->name());
            //_tabWidget->inse
            // TODO: insert tab depending on market cap

            _pages.insert(assetID,page);

            //QToolButton*        testButton=new QToolButton();
            //testButton->setText("FCS");
            //void QTabBar::setTabButton(pageIndex, QTabBar::LeftSide, testButton)
            QCheckBox*            checkBox=new QCheckBox();
            _tabWidget->tabBar()->setTabButton(pageIndex, QTabBar::LeftSide, checkBox);
            //_tabWidget->tabBar()->setTabTextColor(pageIndex,QColor(Qt::red));
            //_tabWidget->set
        }
        _tabWidget->setCurrentWidget(_pages[assetID]);
    }

    //tabWidget->addTab(new QWidget(),"Bitcoin");
}

//
void                    CommunityWidget::removeUnpinnedTabs()
{
    bool removed;
    do
    {
        removed=false;
        for(int ipage=0;ipage<_tabWidget->count();ipage++)
        {
            QWidget*        button      =_tabWidget->tabBar()->tabButton(ipage,QTabBar::LeftSide);
            QCheckBox*      checkBox    =dynamic_cast<QCheckBox*>(button);
            if(checkBox->checkState()!=Qt::Checked)
            {
                onTabCloseRequested(ipage);
                removed=true;
                break;
            }
        }
    }
    while(removed);
}

//
void                    CommunityWidget::onTabCloseRequested(int index)
{
    QWidget*            page    =_tabWidget->widget(index);
    QString             assetID =page->property(PROP_ASSET_ID).toString();
    _tabWidget->removeTab(index);
    _pages.remove(assetID);
    page->deleteLater();
}
