#ifndef MARKETDATACANDLES_H
#define MARKETDATACANDLES_H
#include <QDateTime>
#include <QVector>
#include "FinChartData.h"

class QPaintEvent;

class FinChartWidget;

class FinChartDataCandles : public FinChartData
{
    Q_OBJECT
public:
    explicit FinChartDataCandles(FinChartWidget* chart);
    virtual ~FinChartDataCandles();

    virtual void            update(bool invalidate);
    virtual QDateTime       startTime();
    virtual QDateTime       endTime();
    virtual bool            currentTime(QDateTime&,int& snapX);
    virtual int             interval();
    virtual int             minIntervalWidth();
    virtual bool            getLimits(double& low,double& high,double& last);
    virtual bool            getVolumeLimits(double& low,double& high,double& last);
    virtual bool            getIndicatorLimits(double& low,double& high,double& last);

    virtual void            paintEvent(QPaintEvent* evt);


    struct Entry
    {
        QDateTime   startTime;
        QDateTime   endTime;
        double      open;
        double      close;
        double      high;
        double      low;
        double      volume;
        int         trades;
    };
    struct VisibleEntry
    {
        VisibleEntry(const Entry& e) {this->e=e;tradeSignal=0;}
        VisibleEntry() {}
        Entry       e;
        double      sma[3];
        double      vma[2];
        double      vwma[3];
        //double      ema[3];
        double      macd;
        double      signal;
        double      bollingerAvg;
        double      bollingerDev;
        int         tradeSignal;
    };


    void                    clear();
    void                    setInterval(int intervalSeconds);
    int                     entryCount();
    Entry                   entryAt(int index);
    void                    setEntry(const Entry&);
    bool                    isLatestTime();
    void                    setBollingerBands(bool enable,int history,double multiplier);
    void                    setMovingAverages(int index,bool enable,int history);
    void                    setVolumeMovingAverages(int index,bool enable,int history);
    void                    setIndicatorMACD(int shortHistory,int longHistory,int signalHistory);

protected:
    void                    drawCandles(QPainter* painter);
    void                    drawMovingAverages(QPainter* painter);
    void                    drawVolumeMovingAverages(QPainter* painter);
    void                    drawBollingerBands(QPainter* painter);
    void                    drawVolumes(QPainter* painter);
    void                    drawIndicators(QPainter* painter);

private:
    QVector<Entry>*         _entries;
    QVector<VisibleEntry>*  _visibleEntries;
    bool                    _updated;
    double                  _low;
    double                  _high;
    double                  _volumeLow;
    double                  _volumeHigh;
    double                  _indicatorLow;
    double                  _indicatorHigh;
    QDateTime               _startTime;
    QDateTime               _endTime;
    QDateTime               _currentTime;
    int                     _currentTimeX;
    int                     _interval;
    int                     _candleWidth;
    int                     _candleSpacing;
    // bollinger bands
    bool                    _bollingerBandsEnabled;
    int                     _bollingerBandsHistory;
    double                  _bollingerBandsMultiplier;
    // sma
    bool                    _SMAEnabled[3];
    int                     _SMAHistory[3];
    // vma
    bool                    _VMAEnabled[2];
    int                     _VMAHistory[2];
    // macd
    int                     _macdShortHistory;
    int                     _macdLongHistory;
    int                     _macdSignalHistory;
};

#endif // MARKETDATACANDLES_H
