#ifndef MARKETWATCHLISTWIDGET_H
#define MARKETWATCHLISTWIDGET_H

//
#include <QScrollArea>

//
class MarketWatchListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MarketWatchListWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*);

signals:

public slots:
};

#endif // MARKETWATCHLISTWIDGET_H
