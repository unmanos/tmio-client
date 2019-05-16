#include "MarketWatchListWidget.h"
#include <QPainter>

//
#define TAG "exchanges.MarketWatchListWidget"

//
MarketWatchListWidget::MarketWatchListWidget(QWidget *parent) : QWidget(parent)
{
    resize(width(),1024);
}

//
void                    MarketWatchListWidget::paintEvent(QPaintEvent* /*evt*/)
{
    QPainter painter(this);
    painter.setPen(QColor(255,0,0));
    painter.setBrush(QBrush(QColor(255,0,0)));
    painter.drawLine(QPoint(0,0),QPoint(width(),height()));

}
