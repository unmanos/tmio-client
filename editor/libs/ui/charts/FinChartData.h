#ifndef FINCHARTDATA_H
#define FINCHARTDATA_H

#include <QObject>
#include <QRect>

//
class FinChartWidget;
class QPainter;
class QPaintEvent;
class QMouseEvent;

//
class FinChartData : public QObject
{
    Q_OBJECT
public:
    explicit FinChartData(FinChartWidget* chart);

    struct PaintEvent
    {
        QPainter*       painter;
    };

    virtual void        paintEvent(QPaintEvent* evt)        =0;
    virtual bool        mouseMoveEvent(QMouseEvent* /*event*/)                      { return(false); }

    virtual void        update(bool invalidate)                                     =0;
    virtual QDateTime   startTime()                                                 =0;
    virtual QDateTime   endTime()                                                   =0;
    virtual bool        currentTime(QDateTime&,int& snapX)                          =0;
    virtual int         interval()                                                  =0;
    virtual int         minIntervalWidth()                                          =0;
    virtual bool        getLimits(double& low,double& high, double& last)           =0;
    virtual bool        getVolumeLimits(double& low,double& high, double& last)     =0;
    virtual bool        getIndicatorLimits(double& low,double& high, double& last)  =0;


signals:
    void                changed(FinChartData*);

public slots:

protected:
    FinChartWidget*     _chart;

};

#endif // FINCHARTDATA_H
