#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <QWidget>
#include <QLabel>
#include <QElapsedTimer>
#include <QTime>

// TODO: http://www.modulusfe.com/products/indicators/


class FinChartData;
class FinChartDataCandles;

class FinChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FinChartWidget(QWidget *parent = 0);
    virtual ~FinChartWidget();

    struct              Style
    {
        int volumeHeight;
        int indicatorHeight;
        int marginX;
        int marginTop;
        int marginBottom;
        int separatorHeight;
        int scaleWidth;
        Style()
        {
            volumeHeight        =200;
            indicatorHeight     =100;
            marginX             =8;
            marginTop           =1;
            marginBottom        =30;
            separatorHeight     =10;
            scaleWidth          =100;
        }
    };

    FinChartDataCandles*addCandles();
    void                invalidate();
    QRect               chartRect();
    QRect               volumeRect();
    QRect               indicatorRect();
    QDateTime           startTime();
    QDateTime           endTime();
    QDateTime           cursorTime();

    void                updateTimeRange(bool latest);
    void                updateChart(bool invalidate);
    void                updateLayouts(const QSize* size);
    void                paintSticker(QPainter* painter,QColor color,QPoint p,Qt::Orientation orientation,QString text);
    void                paintPriceScale(QPainter* painter,double low,double current,double high);
    void                paintVolumeScale(QPainter* painter,double low,double current,double high);
    void                paintIndicatorScale(QPainter* painter,double low,double current,double high);
    void                paintTimeScale(QPainter* painter);

protected:
    void                paintEvent(QPaintEvent*);
    void                wheelEvent(QWheelEvent*);
    void                mousePressEvent(QMouseEvent*);
    void                mouseReleaseEvent(QMouseEvent*);
    void                mouseMoveEvent(QMouseEvent*);
    void                resizeEvent(QResizeEvent*);
    void                showEvent(QShowEvent *event);

private:
    enum DragMode
    {
        dragNone,
        dragTimeline,
        dragVolumeUp,
        dragVolumeDown
    };

signals:
    void timeRangeChanged(const QDateTime& start,const QDateTime& end,bool interactive);

public slots:
    void onStep();

private:
    bool                    _needRedraw;
    QList<FinChartData*>    _datas;
    QRect                   _chartRect;
    QRect                   _volumeRect;
    QRect                   _indicatorRect;
    QDateTime               _startTime;
    QDateTime               _endTime;
    QDateTime               _cursorTime;
    int                     _cursorX;
    int                     _cursorY;
    bool                    _updated;

    QDateTime               _gripStartTime;
    QDateTime               _gripEndTime;
    QDateTime               _gripTime;
    QPoint                  _gripCursor;
    DragMode                _dragMode;
    int                     _gripHeight;

    // style
    Style                   _style;
};

#endif // MAPEDITOR_H
