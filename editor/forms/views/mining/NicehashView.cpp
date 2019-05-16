#include <QFrame>
#include <QBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QHeaderView>

#include <md/Asset.h>

#include "NicehashView.h"
#include "Application.h"

//
#define TAG                 "NicehashView"

//
#define API_URL             "https://api.nicehash.com"
#define API_METHOD_STAT     "/api?method=stats.provider"

//
#define COL_ALGO                0
#define COL_SPEED               1
#define COL_ACCEPTED_RATE       2
#define COL_BALANCE             3
#define COL_COUNT               4

//
NicehashView::NicehashView(QString apiAddress,QWidget *parent) : QWidget(parent)
{
    CoinMarketCap*  cmc=Application::instance().cmc();
    // algos
    _algos[0]="Scrypt";
    _algos[1]="SHA256";
    _algos[2]="ScryptNf";
    _algos[3]="X11";
    _algos[4]="X13";
    _algos[5]="Keccak";
    _algos[6]="X15";
    _algos[7]="Nist5";
    _algos[8]="NeoScrypt";
    _algos[9]="Lyra2RE";
    _algos[10]="WhirlpoolX";
    _algos[11]="Qubit";
    _algos[12]="Quark";
    _algos[13]="Axiom";
    _algos[14]="Lyra2REv2";
    _algos[15]="ScryptJaneNf16";
    _algos[16]="Blake256r8";
    _algos[17]="Blake256r14";
    _algos[18]="Blake256r8vnl";
    _algos[19]="Hodl";
    _algos[20]="DaggerHashimoto";
    _algos[21]="Decred";
    _algos[22]="CryptoNight";
    _algos[23]="Lbry";
    _algos[24]="Equihash";
    _algos[25]="Pascal";
    _algos[26]="X11Gost";
    _algos[27]="Sia";
    _algos[28]="Blake2s";
    _algos[29]="Skunk";
    _algos[30]="CryptoNightV7";
    _algos[31]="CryptoNightHeavy";
    _algos[32]="Lyra2Z";
    _algos[33]="X16R";
    // init
    _apiAddress=apiAddress;
    _statsRequest=NULL;
    _lastPaymentsAmount=-1;
    _pendingBalance=-1;
    connect(cmc,SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));
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
    // add pending balance
    {
        _labelPendingBalance = new QLabel();
        _labelPendingBalanceFiat = new QLabel();
        topFormLayout->addWidget(_labelPendingBalance);
        topFormLayout->addSpacing(20);
        topFormLayout->addWidget(_labelPendingBalanceFiat);
        topFormLayout->addStretch(1);
    }
    // add confirmed balance
    {
        _labelLastPaymentsAmount = new QLabel();
        _labelLastPaymentsAmountFiat = new QLabel();
        topFormLayout->addWidget(_labelLastPaymentsAmount);
        topFormLayout->addSpacing(20);
        topFormLayout->addWidget(_labelLastPaymentsAmountFiat);
        topFormLayout->addStretch(1);
    }
    // main frame
    //QFrame*             mainFrame=new QFrame();
    //layout()->addWidget(mainFrame);
    //mainFrame->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    // algorythms
    _treeView           =new QTreeView(this);
    _treeView->setSortingEnabled(true);
    _treeView->setAlternatingRowColors(true);
    _treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _sortFilter=new GenericSortFilterProxyModel(_treeView);
    _algosModel          =new QStandardItemModel();
    //_model->setHorizontalHeaderItem(COL_RANK, new QStandardItem(tr("Rank")));
    _algosModel->setHorizontalHeaderItem(COL_ALGO, new QStandardItem(tr("Algorithm")));
    _algosModel->setHorizontalHeaderItem(COL_SPEED, new QStandardItem(tr("Speed")));
    _algosModel->setHorizontalHeaderItem(COL_ACCEPTED_RATE, new QStandardItem(tr("Accepted")));
    _algosModel->setHorizontalHeaderItem(COL_BALANCE, new QStandardItem(tr("Balance")));
    _sortFilter->setSourceModel(_algosModel);
    _treeView->setModel(_sortFilter);
    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(false);
    _treeView->setHeader(listHeader);
    //listHeader->setSortIndicator(0,Qt::SortOrder::AscendingOrder);
    listHeader->setSectionsMovable(true);
    listHeader->resizeSection(COL_ALGO,200);
    listHeader->resizeSection(COL_SPEED,200);
    listHeader->resizeSection(COL_ACCEPTED_RATE,200);
    listHeader->resizeSection(COL_BALANCE,110);
    _treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    _treeView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    layout()->addWidget(_treeView);
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,SIGNAL(timeout()),this,SLOT(onUpdate()));
    updateTimer->start(1000*30);
    onUpdate();
}

//
QString                 NicehashView::formatHashRate(quint64 rate)
{
    if(rate>=1000000000)
    {
        // Gh/s
        return(QString::number(rate/1000000000,'f',2)+" Gh/s");
    }
    if(rate>=1000000)
    {
        // Mh/s
        return(QString::number(rate/1000000,'f',2)+" Mh/s");
    }
    if(rate>=1000)
    {
        // Mh/s
        return(QString::number(rate/1000,'f',2)+" Kh/s");
    }
    return(QString::number(rate)+" h/s");
}

//
void                    NicehashView::updateUI()
{
    CoinMarketCap*  cmc=Application::instance().cmc();
    double amount[]={_lastPaymentsAmount,_pendingBalance};
    QString text[]={tr("Last 7 days payments"),tr("Pending balance")};
    QLabel* labels[]={_labelLastPaymentsAmount,_labelPendingBalance};
    QLabel* labelsFiat[]={_labelLastPaymentsAmountFiat,_labelPendingBalanceFiat};
    for(int i=0;i<2;i++)
    {
        {
            QString rt;
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg(text[i]);
            rt+="<br>";
            rt+=QString("<span style=\"font-size:11pt;\"><b>%1 </b></span>").arg(cmc->formatFiatCurrency(amount[i],md::Asset::BTC));
            //rt+=QString("<span style=\"font-size:9pt;\">%1</span>").arg(_currencyTicker);
            rt+="</div>";
            labels[i]->setText(rt);
        }
        {
            QString rt;
            double  price=amount[i];
            cmc->assetPrice(cmc->quoteAssetID(),md::Asset::BTC,price,nullptr);
            QString fiatPriceStr=cmc->formatFiatCurrency(price,cmc->quoteAssetID());
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("&nbsp;");
            rt+="<br>";
            rt+=QString("<span style=\"font-size:11pt;\">%1</span>").arg(fiatPriceStr);
            rt+="</div>";
            labelsFiat[i]->setText(rt);
        }
    }
    // update hash algo tree
    foreach(AlgoStat as,_algoStats)
    {
        QString algo=_algos.contains(as.algo)?_algos[as.algo]:QString("Undefined %1").arg(as.algo);
        QList<QStandardItem*> row=_algosModel->findItems(algo,Qt::MatchFlag::MatchExactly,COL_ALGO);
        QStandardItem* item=row.length()?row.at(0):nullptr;
        if(!item)
        {
            item=new QStandardItem(algo);
            row.append(item);
            for(int i=1;i<COL_COUNT;i++)
            {
                row.append(new QStandardItem());
            }
            _algosModel->appendRow(row);
        }
        double speed=as.acceptedSpeed+as.rejectedSpeed;
        double acceptedRate=speed?(as.acceptedSpeed/speed):1;
        _algosModel->item(item->row(),COL_SPEED)->setText(formatHashRate(speed));
        _algosModel->item(item->row(),COL_SPEED)->setData(speed,USER_ROLE_SORT_NUMERIC);
        _algosModel->item(item->row(),COL_ACCEPTED_RATE)->setText(QString::number(acceptedRate*100,'f',2)+"%");
        _algosModel->item(item->row(),COL_ACCEPTED_RATE)->setData(acceptedRate,USER_ROLE_SORT_NUMERIC);
        _algosModel->item(item->row(),COL_BALANCE)->setText(cmc->formatFiatCurrency(as.balance,md::Asset::BTC));
        _algosModel->item(item->row(),COL_BALANCE)->setData(as.balance,USER_ROLE_SORT_NUMERIC);
    }
}

//
void                    NicehashView::onUpdate()
{
    TM_TRACE(TAG,"Update nicehash balance");
    if(!_statsRequest)
    {
        //
        QString url=QString("%1%2&addr=%3").arg(API_URL).arg(API_METHOD_STAT).arg(_apiAddress);
        _statsRequest=new QNetworkRequest(QUrl(url));
        QNetworkReply*      reply=Application::instance().connection().networkManager()->get(*_statsRequest);
        connect(reply, SIGNAL(finished()), this, SLOT(onReceiveStats()));
    }
}

//
void                    NicehashView::onReceiveStats()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        TM_TRACE(TAG,"Received nicehash balance");
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            QJsonObject     root=doc.object();
            if(root.contains("result"))
            {
                QJsonObject result=root["result"].toObject();
                if(result.contains("payments"))
                {
                    QJsonArray      payments=result["payments"].toArray();
                    _lastPaymentsAmount=0;
                    foreach(QJsonValue payment, payments)
                    {
                        QJsonObject paymentObject=payment.toObject();
                        _lastPaymentsAmount+=paymentObject["amount"].toString().toDouble();
                    }
                }
                if(result.contains("stats"))
                {
                    QJsonArray      stats=result["stats"].toArray();
                    _pendingBalance=0;
                    _algoStats.clear();
                    foreach(QJsonValue stat, stats)
                    {
                        QJsonObject statObject=stat.toObject();
                        AlgoStat    as;
                        as.balance         =statObject["balance"].toString().toDouble();
                        as.algo            =statObject["algo"].toInt();
                        as.rejectedSpeed   =statObject["rejected_speed"].toString().toDouble()*1000000000.0;
                        as.acceptedSpeed   =statObject["accepted_speed"].toString().toDouble()*1000000000.0;
                        _pendingBalance+=as.balance;
                        _algoStats.append(as);
                    }
                }
            }
            TM_TRACE(TAG,raw.toStdString().c_str());
        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed nicehash balance (%1)").arg(statusCode));
    }

    delete(_statsRequest);
    _statsRequest=nullptr;
    updateUI();
}

//
void                    NicehashView::onQuoteAssetChanged()
{
    updateUI();
}
