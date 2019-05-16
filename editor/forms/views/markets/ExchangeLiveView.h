#ifndef EXCHANGELIVEVIEW_H
#define EXCHANGELIVEVIEW_H
#include <QWidget>
#include <QLabel>
#include <QTreeView>
#include <QStandardItemModel>
#include <md/MarketData.h>
#include <md/ExchangeFeed.h>
//
#include "libs/ui/sort/GenericSortFilterProxyModel.h"
#include "libs/md/cmc/CoinMarketCap.h"


//
class ExchangeLiveView : public QWidget
{
    Q_OBJECT
public:
    explicit ExchangeLiveView(const QString& exchange,QWidget *parent = nullptr);

signals:

public slots:
    void                        onUpdate();
    void                        onQuoteAssetChanged();
    void                        onCurrentRowChanged(QModelIndex index,QModelIndex index2);
    void                        onOpenTicker(QModelIndex index);
    void                        onTickerChanged(const tmio::md::ExchangeFeed::Ticker& ticker);

private:
    void                        updateData(bool resetChange);

private:
    QString                     _exchange;
    tmio::md::MarketData*       _marketData;
    tmio::md::ExchangeFeed*     _feed;
    QTreeView*                  _treeView;
    GenericSortFilterProxyModel*_sortFilter;
    QStandardItemModel*         _model;
};

#endif // EXCHANGELIVEVIEW_H
