#include "BitcoinBlockExplorerView.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "forms/widgets/BitcoinBlocksWidget.h"

// https://data.bitcoinity.org

//
BitcoinBlockExplorerView::BitcoinBlockExplorerView(QWidget *parent) : QWidget(parent)
{
    // layout
    setLayout(new QVBoxLayout());
    setWindowTitle(tr("Nicehash monitor"));
    // title frame
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    QFrame*             titleFrame=new QFrame();
    titleFrame->setLayout(topFormLayout);
    topFormLayout->setMargin(0);
    topFormLayout->addSpacing(8);
    layout()->addWidget(titleFrame);
    // add coin icon
    {
        QLabel*             coinIcon=new QLabel();
        coinIcon->setPixmap(QIcon(":/icons/nicehash.png").pixmap(30,30));
        topFormLayout->addWidget(coinIcon);
        //topFormLayout->addStretch(1);
    }
    QTabWidget*         mainTabWidget=new QTabWidget();
    mainTabWidget->addTab(new BitcoinBlocksWidget(),"Blocks");
    mainTabWidget->addTab(new QLabel(),"Transactions");
    mainTabWidget->addTab(new QLabel(),"Difficulty");
    mainTabWidget->addTab(new QLabel(),"Miners");
    mainTabWidget->addTab(new QLabel(),"Watch addresses");
    layout()->addWidget(mainTabWidget);
}
