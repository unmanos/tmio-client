#ifndef TOOLBOX_H
#define TOOLBOX_H
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStandardItemModel>
#include <rest/Connection.h>

namespace Ui {
class ToolBox;
}

//
class MarketWatchWidget;

//
class ExchangeFeedsWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit ExchangeFeedsWidget(QWidget *parent = 0);
    ~ExchangeFeedsWidget();

private:

public slots:
    void                onOpenTicker(QString,QString,QString);
    void                onCurrentExchangeChanged(int);

signals:
    void                sigActionTriggered(QAction*);

private:
    struct              Feed
    {
        QString             exchange;
        QString             name;
        MarketWatchWidget*  widget;
    };

    Ui::ToolBox         *ui;
    QBoxLayout*         _layout;
    QList<Feed>         _feeds;
};

#endif // TOOLBOX_H
