#ifndef COINMARKETCAPVIEW_H
#define COINMARKETCAPVIEW_H
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMap>
#include "libs/ui/sort/GenericSortFilterProxyModel.h"

//
class CoinMarketCapView : public QWidget
{
    Q_OBJECT
public:
    explicit CoinMarketCapView(QWidget *parent = nullptr);

    void                            updateData();

signals:

public slots:
    void                            onUpdateData();
    void                            onQuoteAssetChanged();
    void                            onCurrentRowChanged(QModelIndex,QModelIndex);

private:
    QTreeView*                      _treeView;
    QStandardItemModel*             _model;
    QMap<int,QStandardItem*>        _lookup;
    GenericSortFilterProxyModel*    _sortFilter;
};

#endif // COINMARKETCAPVIEW_H
