#include "Application.h"
#include "LoadingDialog.h"
#include "ui_LoadingDialog.h"
#include <QTimer>
#include <QMovie>

#include <md/MarketData.h>
#include <md/Exchange.h>
#include <md/Asset.h>

//
using namespace tmio;

//
LoadingDialog::LoadingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingDialog)
{
    ui->setupUi(this);
    // timer
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    // wait movie
    QMovie *movie = new QMovie(":/loading/spin.gif");
    movie->setScaledSize(ui->wait->size());
    ui->wait->setMovie(movie);
    ui->wait->setAlignment(Qt::AlignCenter);
    movie->start();
    // init cmc
    //Application::instance().cmc();
    // check timer
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onCheckReady()));
    timer->start(250);
}

//
LoadingDialog::~LoadingDialog()
{
    delete ui;
    //accept();
}

void                LoadingDialog::nextPhase()
{
    if(_phase==0)
    {
        // load donate assets icons
        Application::instance().cmc()->assetIcon("bitcoin");
        Application::instance().cmc()->assetIcon("litecoin");
        Application::instance().cmc()->assetIcon("dash");
        // load exchanges quote assets icons
        md::MarketData*     marketData  =Application::instance().marketData();
        foreach(QString exchangeID,marketData->exchanges())
        {
            md::Exchange*   exchange    =marketData->exchange(exchangeID);
            foreach(QString quoteAssetID,exchange->quoteAssets())
            {
                Application::instance().cmc()->assetIcon(quoteAssetID);
            }
        }
        QStringList          watches    =Application::instance().marketData()->watchAssets();
        foreach(QString assetID,watches)
        {
            Application::instance().cmc()->assetIcon(assetID);
        }
    }
    if(_phase==1)
    {
        accept();
    }
    _phase++;
}

//
void                LoadingDialog::onCheckReady()
{
    Application::instance().newsFeed();
    if( (Application::instance().cmc()->ready())&&(Application::instance().marketData()->ready()) )
    {
        nextPhase();
    }
}
