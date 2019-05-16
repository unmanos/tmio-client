#ifndef PIECHART_H
#define PIECHART_H
#include <QWidget>
#include <QMap>
#include <QTimer>
#include <QtMath>
#include <QIcon>

//
class PieChart : public QWidget
{
    Q_OBJECT
public:
    PieChart(QWidget *parent = nullptr);
    virtual ~PieChart();

    void                    resetValues();
    void                    setValue(const QString& id,const QString& name,double amount);
    void                    setValue(const QString& id,const QString& name,QIcon icon,double amount);
    double                  posToAngle(const QPoint&,double* plength=nullptr);

protected:
    void                    updateDrawValues();

    void                    paintEvent(QPaintEvent*);
    void                    wheelEvent(QWheelEvent*);
    void                    mousePressEvent(QMouseEvent*);
    void                    mouseReleaseEvent(QMouseEvent*);
    void                    mouseMoveEvent(QMouseEvent*);
    void                    resizeEvent(QResizeEvent*);
    void                    showEvent(QShowEvent *event);

signals:

public slots:
    void                    onUpdate();

private:
    struct                  Value
    {
        QString         id;
        QString         name;
        QColor          color;
        double          amount;
        QIcon           icon;
        double          angle;
        double          dist;
    };
    QList<Value>            _values;
    double                  _total              =0;
    QString                 _activeValue;
    QTimer*                 _updateTimer        =nullptr;
    double                  _initAngle          =M_PI+M_PI/2.0;
    int                     _margin             =80;
    bool                    _grip               =false;
    double                  _gripInitAngle      =0;
    double                  _gripAngle;
    QList<QColor>           _defaultColors;
    bool                    _upToDate           =false;
    double                  _innerRadius        =0.6;
};

#endif // PIECHART_H
