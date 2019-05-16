#ifndef LINECHART_H
#define LINECHART_H
#include <QWidget>
#include <QVariant>
#include <QMap>

//
class LineChart : public QWidget
{
    Q_OBJECT
public:
    explicit LineChart(QWidget *parent = nullptr);

    enum eSide
    {
        Left,
        Right
    };

    enum eAxe
    {
        Vertical,
        Horizontal
    };

    typedef QString(*FormatFn)(eSide side,eAxe axe,QVariant value);

    void                    addDataSet(const QString& name,eSide side);
    bool                    setDataPoint(const QString& setName,QVariant x,QVariant y);
    void                    setHorizontalUnits(QVariant::Type type);
    void                    setVerticalUnits(eSide side,QVariant::Type type);
    void                    setFormatter(FormatFn);
    //void                    setVerticalUnits(QVariant::Type type);

protected:
    struct                  DataSet
    {
        eSide                   side;
        QString                 name;
        QVariant                xmin;
        QVariant                xmax;
        QVariant                ymin;
        QVariant                ymax;
        QMap<QVariant,QVariant> points;
        QVector<QPointF>        spoints;
        QColor                  color;
    };

    void                    updateDataSets();
    QString                 formatValue(eSide side,eAxe axe,QVariant value);
    double                  getWeight(const QVariant& min,const QVariant& value,const QVariant& max);
    QList<QPointF>          getDataSetPoints(const DataSet& dataset);
    void                    updateRects();
    void                    paintDataSets(QPainter&);
    void                    paintVerticalScales(QPainter&);
    void                    paintEvent(QPaintEvent*);
    void                    wheelEvent(QWheelEvent*);
    void                    mousePressEvent(QMouseEvent*);
    void                    mouseReleaseEvent(QMouseEvent*);
    void                    mouseMoveEvent(QMouseEvent*);
    void                    resizeEvent(QResizeEvent*);
    void                    showEvent(QShowEvent *event);

signals:

public slots:

private:
    QMap<QString,DataSet>   _dataSets;
    QRect                   _chartRect;
    QRect                   _chartFrameRect;
    bool                    _dataSetsUpdated            =false;
    QVariant::Type          _horizontalUnitsType;
    QVariant::Type          _verticalUnitsType[2];
    QVariant                _xmin;
    QVariant                _xmax;
    QVariant                _ymin[2];
    QVariant                _ymax[2];
    QList<QColor>           _defaultColors;
    QString                 _selectedSet;
    int                     _selectedPoint              =-1;
    QPoint                  _cursorPos;
    FormatFn                _formatFn                   =nullptr;
    int                     _horizontalMargin           =5;
};

#endif // LINECHART_H
