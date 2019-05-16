#include "TickerPriceView.h"
#include "ui_TickerPriceView.h"
#include "Application.h"

#include <QTimer>
#include <QComboBox>
#include <QTime>
#include <QPushButton>

#include "libs/ui/charts/FinChartWidget.h"
#include "libs/ui/charts/FinChartDataCandles.h"
#include "ChartSettingsPopup.h"

#include <md/Asset.h>

//
#define TAG "TickerPriceView"

#define API_REQUEST_BINARY      true
#if(API_REQUEST_BINARY)
    #define API_REQUEST_UPDATE      "marketdata/exchanges/%1/tickers/%2/candlesticks?binary=true&count=3&interval=%3"
    #define API_REQUEST_HISTORY     "marketdata/exchanges/%1/tickers/%2/candlesticks?binary=true&count=%3&interval=%4"
#else
    #define API_REQUEST_UPDATE      "marketdata/exchanges/%1/tickers/%2/candlesticks?count=3&interval=%3"
    #define API_REQUEST_HISTORY     "marketdata/exchanges/%1/tickers/%2/candlesticks?count=%3&interval=%4"
#endif

//
TickerPriceView::TickerPriceView(QString exchange,QString assetID,QString pair,QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TickerPriceView)
{
    ui->setupUi(this);
    //ui->mainLayout->setContentsMargins(0,0,0,0);
    _assetID=assetID;
    _exchange=exchange;
    _changed=true;

    tmio::md::ExchangeFeed*         feed        =nullptr;
    tmio::md::MarketData*           marketData  =Application::instance().marketData();

    connect(Application::instance().cmc(),SIGNAL(quoteAssetChanged()),this,SLOT(onQuoteAssetChanged()));

    _chartSettingPopup = nullptr;
    _pair =pair;
    {
        QStringList list=_pair.split("-");
        _assetTicker        =list.first();
        _quoteTicker    =list.last();
        _quoteID        =marketData->loopupExchangeAsset(exchange,_quoteTicker);
        feed=marketData->exchangeFeed(exchange);
        connect(feed,SIGNAL(tickerChanged(const tmio::md::ExchangeFeed::Ticker&)),this,SLOT(onTickerChanged(const tmio::md::ExchangeFeed::Ticker&)));
    }

    md::Asset*          asset=Application::instance().marketData()->asset(assetID);
    _historyCount       =400;
    _firstUpdate        =true;
    _historyStart       =QDateTime::currentDateTime();
    _historyEnd         =_historyStart.addDays(1);
    TM_TRACE(TAG,QString("Open pair price view: %1").arg(_pair));
    // title
    setWindowTitle(QString("%1 / Ticker %2").arg(exchange.toUpper().left(1)+exchange.right(exchange.length()-1)).arg(pair));
    //setWindowIcon(Application::instance().cmc()->icon(coinID));
    // setup top frame
    QHBoxLayout*        topFormLayout=new QHBoxLayout();
    QFrame*             titleFrame=new QFrame();
    titleFrame->setLayout(topFormLayout);
    topFormLayout->setMargin(0);
    topFormLayout->addSpacing(8);
    if(asset)
    {
        // add coin icon
        {
            QLabel*             coinIcon=new QLabel();
            coinIcon->setPixmap(asset->icon().pixmap(30,30));
            topFormLayout->addWidget(coinIcon);
            //topFormLayout->addStretch(1);
        }
        // add pair info
        {
            QString rt;
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;\">";
            rt+=QString("<span style=\"font-size:11pt;\"><b>%1</b></span>").arg(_assetTicker);
            rt+=QString("<span style=\"font-size:9pt;\"> / %1</span>").arg(_quoteTicker);
            rt+="<br>";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg(asset->name());
            rt+="</div>";

            QLabel *label = new QLabel();
            label->setText(rt);
            topFormLayout->addWidget(label);
            topFormLayout->addStretch(1);
        }
    }
    // add last price
    {
        _labelLastPrice = new QLabel();
        _labelLastPriceFiat = new QLabel();
        topFormLayout->addWidget(_labelLastPrice);
        topFormLayout->addSpacing(20);
        topFormLayout->addWidget(_labelLastPriceFiat);
        topFormLayout->addStretch(1);
    }

    // add interval combo
    {
        QLabel*             titleLabel=new QLabel();
        titleLabel->setText("Interval");
        _intervalCombo=new QComboBox();
        _intervalCombo->addItem(tr("1m"),QVariant(1));
        _intervalCombo->addItem(tr("2m"),QVariant(2));
        _intervalCombo->addItem(tr("3m"),QVariant(3));
        _intervalCombo->addItem(tr("5m"),QVariant(5));
        _intervalCombo->addItem(tr("10m"),QVariant(10));
        _intervalCombo->addItem(tr("15m"),QVariant(15));
        _intervalCombo->addItem(tr("20m"),QVariant(20));
        _intervalCombo->addItem(tr("30m"),QVariant(30));
        _intervalCombo->addItem(tr("1h"),QVariant(60*1));
        _intervalCombo->addItem(tr("2h"),QVariant(60*2));
        _intervalCombo->addItem(tr("3h"),QVariant(60*3));
        _intervalCombo->addItem(tr("4h"),QVariant(60*4));
        _intervalCombo->addItem(tr("6h"),QVariant(60*6));
        _intervalCombo->addItem(tr("8h"),QVariant(60*8));
        _intervalCombo->addItem(tr("12h"),QVariant(60*12));
        _intervalCombo->addItem(tr("1d"),QVariant(60*24*1));
        _intervalCombo->addItem(tr("2d"),QVariant(60*24*2));
        _intervalCombo->addItem(tr("3d"),QVariant(60*24*3));
        _intervalCombo->addItem(tr("4d"),QVariant(60*24*4));
        _intervalCombo->addItem(tr("5d"),QVariant(60*24*5));
        _intervalCombo->addItem(tr("6d"),QVariant(60*24*6));
        _intervalCombo->addItem(tr("1w"),QVariant(60*24*7));
        _intervalCombo->setCurrentIndex(4);
        _interval = _intervalCombo->currentData().toInt();
        connect(_intervalCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onIntervalChanged(int)));
        topFormLayout->addWidget(titleLabel);
        topFormLayout->addWidget(_intervalCombo);
    }
    // add settings button
    {
        QPushButton*       settingsButton=new QPushButton();
        settingsButton->setText(tr("Settings"));
        topFormLayout->addWidget(settingsButton);
        connect(settingsButton,SIGNAL(clicked(bool)),this,SLOT(onChartSettings(bool)));
    }
    // add spacer
    topFormLayout->addSpacing(107);


    layout()->addWidget(titleFrame);

    //QHBoxLayout*        chartLayout=new QHBoxLayout();
    //layout()->addItem(chartLayout);

    _updateTimer=nullptr;
    _chart=new FinChartWidget();
    connect(_chart,SIGNAL(timeRangeChanged(QDateTime,QDateTime,bool)),this,SLOT(onTimeRangeChanged(QDateTime,QDateTime,bool)));

    layout()->addWidget(_chart);
    _chart->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _candles=_chart->addCandles();

    // get settings
    _chartSettingPopup=new ChartSettingsPopup(this);
    onUpdateSettings(_chartSettingPopup->settings());
    connect(_chartSettingPopup,SIGNAL(changed(const ChartSettingsPopup::Settings&)),this,SLOT(onUpdateSettings(const ChartSettingsPopup::Settings&)));
    requestData(0);
}

//
TickerPriceView::~TickerPriceView()
{
    delete ui;
}

//
void        TickerPriceView::updateCurrentPrice(double price)
{
#if 1
    CoinMarketCap*      cmc=Application::instance().cmc();
    //double      price=json.value("close").toDouble();
    QString     priceStr        =cmc->formatAmount(price,8);
    QString     fiatPriceStr;
    double      assetPrice      =price;
    if(_quoteID.length())
    {
        if(cmc->assetPrice(cmc->quoteAssetID(),_quoteID,assetPrice,nullptr))
        {
            fiatPriceStr=cmc->formatFiatCurrency(assetPrice,cmc->quoteAssetID());
        }
    }
    else
    {
        // check if we can get price of the asset on the same exchange in BTC
        tmio::md::MarketData*       marketData  =Application::instance().marketData();
        md::ExchangeFeed*           feed        =marketData->exchangeFeed(_exchange);
        md::ExchangeFeed::Ticker    ticker      =feed->ticker(_pair);
        if(ticker.assetID.length())
        {
            assetPrice=1./price;
            if(cmc->assetPrice(cmc->quoteAssetID(),ticker.assetID,assetPrice,nullptr))
            {
                fiatPriceStr=cmc->formatFiatCurrency(assetPrice*price,cmc->quoteAssetID())+" (converted)";
            }
        }
        //ticker.
        //feed->ticker
    }
    {
        QString rt;
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
        rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("Last price");
        //rt+="<br><img src=':/icons/common/chat.png'/>";
        rt+="<br>";
        rt+=QString("<span style=\"font-size:11pt;\"><b>%1 </b></span>").arg(priceStr);
        rt+=QString("<span style=\"font-size:9pt;\">%1</span>").arg(_quoteTicker);
        rt+="</div>";
        _labelLastPrice->setText(rt);
    }

    if(fiatPriceStr.length())
    {
        QString rt;
        rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                        "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
        rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("&nbsp;");
        rt+="<br>";
        rt+=QString("<span style=\"font-size:11pt;\">%1</span>").arg(fiatPriceStr);
        rt+="</div>";
        _labelLastPriceFiat->setText(rt);
    }
#endif
}

//
void        TickerPriceView::requestData(qint64 middleEpoch)
{
    // clean up previous requests
    if(_updateRequest)
    {
        delete(_updateRequest);
        _updateRequest=nullptr;
    }
    if(_historyRequest)
    {
        delete(_historyRequest);
        _historyRequest=nullptr;
    }
    if(_updateTimer)
    {
        delete(_updateTimer);
        _updateTimer=nullptr;
    }
    // fill data
    _updateRequest=Application::instance().connection().createRequest(QString(API_REQUEST_UPDATE).arg(_exchange).arg(_pair).arg(_interval));
    connect(_updateRequest, SIGNAL(finished(tmio::rest::RequestResult&)), this, SLOT(onReceiveCandles(tmio::rest::RequestResult&)));

    QString historyURL=QString(API_REQUEST_HISTORY).arg(_exchange).arg(_pair).arg(_historyCount).arg(_interval);
    if(middleEpoch!=0)
    {
        historyURL+=QString("&middleTime=%1").arg(middleEpoch);
    }
    _historyRequest=Application::instance().connection().createRequest(historyURL);
    connect(_historyRequest, SIGNAL(finished(tmio::rest::RequestResult&)), this, SLOT(onReceiveCandles(tmio::rest::RequestResult&)));
    _historyRequest->get(false);
}

//
void        TickerPriceView::onReceiveCandles(tmio::rest::RequestResult& result)
{
    bool advanceChart=_candles->isLatestTime();
    if(_historyRequest)
    {
        _candles->clear();
    }
    _candles->setInterval(_interval*60);
#if(API_REQUEST_BINARY)
    QDataStream     stream(result._raw);
    stream.setVersion(QDataStream::Qt_4_8);
    int row=0;
    while(!stream.atEnd())
    {
        FinChartDataCandles::Entry    candle;
        quint32             sId,sStartTime,sEndTime;
        double              sOpen,sClose,sHigh,sLow,sVolume,sQuoteVolume;
        quint32             sTrades;
        bool                sIsFinal;
        stream >> sId;
        stream >> sStartTime;
        stream >> sEndTime;
        stream >> sOpen;
        stream >> sClose;
        stream >> sHigh;
        stream >> sLow;
        stream >> sVolume;
        stream >> sQuoteVolume;
        stream >> sTrades;
        stream >> sIsFinal;

        candle.startTime    =QDateTime::fromSecsSinceEpoch(sStartTime);
        candle.endTime      =QDateTime::fromSecsSinceEpoch(sEndTime).addMSecs(999);
        candle.open         =sOpen;
        candle.close        =sClose;
        candle.high         =sHigh;
        candle.low          =sLow;
        candle.volume       =sVolume;
        // update time range only for history
        if(_historyRequest)
        {
            if(row==0)
            {
                _historyStart=candle.startTime;
            }
            if(stream.atEnd())
            {
                _historyEnd=candle.endTime;
            }
        }
        else
        {
            // not history, update price
            if(stream.atEnd())
            {
                updateCurrentPrice(candle.close);
            }
        }
        _candles->setEntry(candle);
        row++;
    }

#else
    QJsonArray entries=result._document.array();
    for(int row=0;row<entries.count();row++)
    {
        QJsonObject json=entries.at(row).toObject();
        //series->append(row, entry.value("close").toDouble());
        //series->append(row, entry.value("close").toDouble());
        //*series << QPointF(row, entry.value("close").toDouble());
        FinChartDataCandles::Entry    candle;
        candle.startTime    =QDateTime::fromString(json.value("startTime").toString(),Qt::ISODate);
        candle.endTime      =QDateTime::fromString(json.value("endTime").toString(),Qt::ISODate);
        candle.open         =json.value("open").toDouble();
        candle.close        =json.value("close").toDouble();
        candle.high         =json.value("high").toDouble();
        candle.low          =json.value("low").toDouble();
        candle.volume       =json.value("volume").toDouble();
        //bool isFinal        =json.value("isFinal").toBool();
        // update time range only for history
        if(_historyRequest)
        {
            if(row==0)
            {
                _historyStart=candle.startTime;
            }
            if(row==(entries.count()-1))
            {
                _historyEnd=candle.endTime;
            }
        }
        else
        {
            // not history, update price
            if(row==(entries.count()-1))
            {
                updateCurrentPrice(candle.close);
            }
        }
        _candles->setEntry(candle);
    }
#endif
    //_chart->updateChart(true);

    if(_historyRequest)
    {
        delete(_historyRequest);
        _historyRequest=nullptr;
    }

    if(!_updateTimer)
    {
        //EG_TRACE(TAG,QString("Start update real time with interval %1").arg(_interval));
        if(_firstUpdate)
        {
            _chart->updateTimeRange(true);
            _chart->updateChart(true);
            _chart->updateTimeRange(true);
            _firstUpdate=false;
        }

        _updateTimer = new QTimer(this);
        connect(_updateTimer, SIGNAL(timeout()), this, SLOT(onRequestUpdate()));
    }
    else
    {
        if(advanceChart)
        {
            //EG_TRACE(TAG,"Advance chart");
            _chart->updateTimeRange(true);
        }
        else
        {
            //EG_TRACE(TAG,"keep chart time");
        }
    }
    // start realtime update
    _updateTimer->start(100);
}

//
void        TickerPriceView::onRequestUpdate()
{
    if(_changed)
    {
        _changed=false;
        _updateTimer->stop();
        _updateRequest->get(false);
    }
}

//
void        TickerPriceView::onIntervalChanged(int index)
{
    TM_TRACE(TAG,"Interval changed");
    _interval=_intervalCombo->itemData(index).toInt();
    if(_updateTimer)
    {
        delete(_updateTimer);
        _updateTimer=nullptr;
    }
    _candles->clear();
    _firstUpdate=true;
    requestData(0);
}

//
void        TickerPriceView::onTimeRangeChanged(const QDateTime& /*start*/,const QDateTime& /*end*/,bool /*interactive*/)
{
    //EG_TRACE(TAG,"Time range changed");
    if( (_candles->entryCount()>=3)&&(!_historyRequest) )
    {
        qint64                          timeMargin=(_interval*60000*100);
        qint64                          chartStart=_chart->startTime().toMSecsSinceEpoch();
        qint64                          chartEnd=_chart->endTime().toMSecsSinceEpoch();
        qint64                          chartMiddle=(chartStart+chartEnd)/2;
        qint64                          entryStart=_historyStart.toMSecsSinceEpoch()+timeMargin;
        qint64                          entryEnd=_historyEnd.toMSecsSinceEpoch();
        qint64                          current=QDateTime::currentMSecsSinceEpoch();

        //if()
        if(entryStart>chartStart)
        {
            TM_TRACE(TAG,"Time range update backward");
            requestData(chartMiddle);
        }
        else if( (entryEnd<chartEnd)&&(chartEnd<current) )
        {
            TM_TRACE(TAG,"Time range update forward");
            requestData(chartMiddle);
        }
    }
    //_candles->startTime()
}

//
void       TickerPriceView::onChartSettings(bool /*clicked*/)
{
    QPushButton*        button=(QPushButton*)sender();
    TM_TRACE(TAG,QString("Open modeless settings dialog (pos=%1, %2)").arg(button->pos().x()).arg(button->pos().y()));
    QPoint              popUpPosition=button->mapToGlobal(button->rect().bottomRight());
    int                 popupWidth=_chartSettingPopup->width();
    popUpPosition.rx()-=popupWidth;
    _chartSettingPopup->resize(popupWidth,_chartSettingPopup->height());

    _chartSettingPopup->move(popUpPosition);
    _chartSettingPopup->setModal(false);
    //_chartSettingPopup->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    _chartSettingPopup->setWindowFlags(Qt::Popup);
    //_chartSettingPopup-
    //_chartSettingPopup->set
    _chartSettingPopup->show();
}

//
void       TickerPriceView::onUpdateSettings(const ChartSettingsPopup::Settings& settings)
{
    TM_TRACE(TAG,"Setting changed");
    _candles->setBollingerBands(settings.bollingerEnable,settings.bollingerHistory,settings.bollingerMultiplier);
    _candles->setMovingAverages(0,settings.smaShortEnable,settings.smaShortHistory);
    _candles->setMovingAverages(1,settings.smaMiddleEnable,settings.smaMiddleHistory);
    _candles->setMovingAverages(2,settings.smaLongEnable,settings.smaLongHistory);
    _candles->setVolumeMovingAverages(0,settings.vmaShortEnable,settings.vmaShortHistory);
    _candles->setVolumeMovingAverages(1,settings.vmaLongEnable,settings.vmaLongHistory);
    _candles->setIndicatorMACD(settings.macdShortHistory,settings.macdLongHistory,settings.macdSignalHistory);
}

//
void      TickerPriceView::onTickerChanged(const tmio::md::ExchangeFeed::Ticker& ticker)
{
    if(ticker.name==_pair)
    {
        TM_TRACE(TAG,"Received update for "+_pair);
        _changed=true;
    }
}

//
void      TickerPriceView::onQuoteAssetChanged()
{
    _changed=true;
}
