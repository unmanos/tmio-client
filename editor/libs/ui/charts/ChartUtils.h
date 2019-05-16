#ifndef CHARTUTILS_H
#define CHARTUTILS_H
#include <QObject>
#include <QPainter>
#include <QWidget>

//
class ChartUtils : public QObject
{
    Q_OBJECT
public:
    explicit ChartUtils(QWidget* widget,QPainter& painter);

    void                paintSticker(const QRect& chartRect,QPoint p,QColor color,Qt::Orientation orientation,bool flip,QString text);

signals:

public slots:

private:
    QPainter&           _painter;
    QWidget*            _widget;
};

#endif // CHARTUTILS_H
