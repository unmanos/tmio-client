#include <QSound>
#include <QTimer>
#include <md/Asset.h>
#include <rest/Request.h>
#include "Application.h"
#include "AboutDialog.h"
#include "ui_AboutDialog.h"

//
#define TAG     "AboutDialog"

//
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    buildDonateFrame();
    buildLicenseFrame();

    ui->tabWidget->setCurrentIndex(0);

    //QSound::play(":/sounds/bell2.wav");
    connect(ui->netResetButton,SIGNAL(clicked()),this,SLOT(onResetNetStats()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onUpdate()));
    timer->start(250);

    QTimer *testTimer = new QTimer(this);
    connect(testTimer, SIGNAL(timeout()), this, SLOT(onV2TestRequest()));
    testTimer->start(500);

    onUpdate();
}

//
AboutDialog::~AboutDialog()
{
    delete ui;
}

//
void                AboutDialog::buildDonateFrame()
{
    QStringList     assets=Application::instance().marketData()->assets();
    QVBoxLayout*    layout=new QVBoxLayout();
    ui->donateFrame->setLayout(layout);
    foreach(QString assetID,assets)
    {
        QString     address=Application::instance().connection().account()->donateAddress(assetID);
        if(address.length())
        {
            md::Asset*      asset=Application::instance().marketData()->asset(assetID);
            QFrame*         addressFrame=new QFrame();
            QHBoxLayout*    mainAddressLayout=new QHBoxLayout();
            addressFrame->setLayout(mainAddressLayout);
            // icon
            QLabel*         iconLabel=new QLabel();
            iconLabel->setPixmap(Application::instance().cmc()->assetIcon(assetID).pixmap(32,32));
            iconLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
            mainAddressLayout->addWidget(iconLabel);
            // form
            QFrame*         infoFrame=new QFrame();
            QFormLayout*    infoLayout=new QFormLayout();
            infoFrame->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
            infoFrame->setLayout(infoLayout);
            mainAddressLayout->addWidget(infoFrame);

            //infoLayout->addWidget(new QLabel(address));
            QLineEdit*      addressText=new QLineEdit(address);
            addressText->setReadOnly(true);
            infoLayout->addRow(new QLabel(asset->name()+" address"),addressText);

            layout->addWidget(addressFrame);
        }
        layout->addStretch(1);
    }
}

//
void                AboutDialog::buildLicenseFrame()
{
    ui->licenseText->setText("Loading...");
    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(QUrl("https://tokenmark.io/pages/terms.html"));
    QNetworkReply*      reply=Application::instance().networkManager()->get(netReq);
    connect(reply, &QNetworkReply::finished, this, &AboutDialog::onReceiveLicense);
}

//
void                AboutDialog::onReceiveLicense()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    if(reply->error() == QNetworkReply::NoError)
    {
        //TM_TRACE(TAG,"Received account");
        int statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            ui->licenseText->setTextFormat(Qt::RichText);
            ui->licenseText->setText(QString(raw));
            ui->licenseText->setTextInteractionFlags(Qt::TextBrowserInteraction);
            ui->licenseText->setOpenExternalLinks(true);
        }
    }
}

//
void                AboutDialog::onUpdate()
{
    rest::Connection&       connection=Application::instance().connection();
    qint64                  count,bytesIn,bytesOut;
    qint64                  runningSeconds=connection.connectedSince().secsTo(QDateTime::currentDateTime());
    double                  seconds=static_cast<double>(runningSeconds);
    if(seconds<1)
    {
        ui->netDownloadRate->setText("?");
        ui->netUploadRate->setText("?");
        ui->netWsDownloadRate->setText("?");
        ui->netWsUploadRate->setText("?");
    }
    // rest
    {
        connection.stats(count,bytesIn,bytesOut);
        ui->netCount->setText(QString::number(count));
        ui->netBytesIn->setText (QLocale::system().toString(bytesIn) +" bytes");
        ui->netBytesOut->setText(QLocale::system().toString(bytesOut)+" bytes");
        if(seconds>=1)
        {
            double                  downloadRate=static_cast<double>(bytesIn)/seconds;
            double                  uploadRate  =static_cast<double>(bytesOut)/seconds;
            ui->netDownloadRate->setText(QLocale::system().toString(downloadRate*0.0078125,'f',1)+" kbps");
            ui->netUploadRate->setText(QLocale::system().toString(uploadRate*0.0078125,'f',1)+" kbps");
        }
    }
    // ws
    {
        connection.wsStats(count,bytesIn,bytesOut);
        ui->netWsCount->setText(QString::number(count));
        ui->netWsBytesIn->setText (QLocale::system().toString(bytesIn) +" bytes");
        ui->netWsBytesOut->setText(QLocale::system().toString(bytesOut)+" bytes");
        if(seconds>=1)
        {
            double                  downloadRate=static_cast<double>(bytesIn)/seconds;
            double                  uploadRate  =static_cast<double>(bytesOut)/seconds;
            ui->netWsDownloadRate->setText(QLocale::system().toString(downloadRate*0.0078125,'f',1)+" kbps");
            ui->netWsUploadRate->setText(QLocale::system().toString(uploadRate*0.0078125,'f',1)+" kbps");
        }
    }
    qint64                  runningHours    =runningSeconds/3600;
    qint64                  runningMinutes  =(runningSeconds%3600)/60;
    if(runningSeconds<60)
    {
        ui->connectionTime->setText(QString("Connected for %1 seconds(s)").arg(runningSeconds));
    }
    else
    {
        ui->connectionTime->setText(QString("Connected for %1 hour(s) and %2 minute(s)").arg(runningHours).arg(runningMinutes));
    }
}

//
void                AboutDialog::onResetNetStats()
{
    rest::Connection&       connection=Application::instance().connection();
    connection.resetStats();
    onUpdate();
}

//
void                AboutDialog::onV2TestRequest()
{
    rest::Connection&       connection=Application::instance().connection();
    if(_v2TestRequest==nullptr)
    {
        _v2TestRequest=connection.createRequest("todo/test",2);
        connect(_v2TestRequest,&rest::Request::finished,this,&AboutDialog::onReceiveV2TestRequest);
        _v2TestRequest->get(false);
    }
}

//
void                AboutDialog::onReceiveV2TestRequest(rest::RequestResult& result)
{
    qDebug() << result._raw;
    delete(_v2TestRequest);
    _v2TestRequest=nullptr;
}
