#include <QLabel>
#include <QTabWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include "Application.h"
#include "CommunityPageAsset.h"

//
#define TAG         "CommunityPageAsset"

//
CommunityPageAsset::CommunityPageAsset(const QString& assetID,QWidget *parent) : QWidget(parent)
{
    TM_TRACE(TAG,"create");
    cm::Subscriber*         subscriber  =Application::instance().communitySubscriber();
    _channel=QString("assets/%1").arg(assetID);
    subscriber->subscribe(_channel);
    //
    QWidget *firstPageWidget = new QWidget;
    QWidget *secondPageWidget = new QWidget;
    QWidget *thirdPageWidget = new QWidget;

    QStackedWidget *stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(firstPageWidget);
    stackedWidget->addWidget(secondPageWidget);
    stackedWidget->addWidget(thirdPageWidget);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(stackedWidget);
    setLayout(layout);

    QComboBox *pageComboBox = new QComboBox;
    pageComboBox->addItem(tr("Page 1"));
    pageComboBox->addItem(tr("Page 2"));
    pageComboBox->addItem(tr("Page 3"));
    connect(pageComboBox, SIGNAL(activated(int)),
            stackedWidget, SLOT(setCurrentIndex(int)));

}

//
CommunityPageAsset::~CommunityPageAsset()
{
    TM_TRACE(TAG,"destroy");
    cm::Subscriber*         subscriber  =Application::instance().communitySubscriber();
    subscriber->unsubscribe(_channel);
}

