#ifndef TICKERPRICEVIEW_H
#define TICKERPRICEVIEW_H

#include <QFrame>
#include <QComboBox>
#include <QLabel>
#include <Core.h>
#include <rest/rest.h>
#include <rest/RequestResult.h>
#include <rest/Request.h>
#include "libs/md/cmc/CoinMarketCap.h"
#include "ChartSettingsPopup.h"

#include "md/MarketData.h"
#include "md/ExchangeFeed.h"


class FinChartWidget;
class FinChartDataCandles;

namespace Ui {
class TickerPriceView;
}

class TickerPriceView : public QFrame
{
    Q_OBJECT

public:
    explicit TickerPriceView(QString exchange,QString assetID,QString pair,QWidget *parent = 0);
    ~TickerPriceView();

protected:
    void                    requestData(qint64 middleEpoch);
    void                    updateCurrentPrice(double price);

public slots:
    void                    onReceiveCandles(tmio::rest::RequestResult&);
    void                    onRequestUpdate();
    void                    onIntervalChanged(int);
    void                    onTimeRangeChanged(const QDateTime& start,const QDateTime& end,bool interactive);
    void                    onChartSettings(bool clicked);
    void                    onUpdateSettings(const ChartSettingsPopup::Settings&);
    void                    onTickerChanged(const tmio::md::ExchangeFeed::Ticker& ticker);
    void                    onQuoteAssetChanged();

private:
    Ui::TickerPriceView*    ui;
    QString                 _exchange;
    QString                 _pair;
    QString                 _assetTicker;
    QString                 _quoteTicker;
    QString                 _assetID;
    QString                 _quoteID;
    QComboBox*              _intervalCombo      =nullptr;
    FinChartWidget*         _chart              =nullptr;
    FinChartDataCandles*    _candles            =nullptr;
    tmio::rest::Request*    _historyRequest     =nullptr;
    tmio::rest::Request*    _updateRequest      =nullptr;
    QTimer*                 _updateTimer        =nullptr;
    int                     _interval;
    int                     _historyCount;
    bool                    _firstUpdate;
    bool                    _changed;
    QDateTime               _historyStart;
    QDateTime               _historyEnd;
    QLabel*                 _labelLastPrice     =nullptr;
    QLabel*                 _labelLastPriceFiat =nullptr;
    ChartSettingsPopup*     _chartSettingPopup  =nullptr;
};

#endif // TICKERPRICEVIEW_H
