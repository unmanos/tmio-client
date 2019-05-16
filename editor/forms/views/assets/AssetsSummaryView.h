#ifndef ASSETSSUMMARYVIEW_H
#define ASSETSSUMMARYVIEW_H
#include <QLabel>
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>

//
#include "libs/ui/sort/GenericSortFilterProxyModel.h"
#include "libs/ui/charts/PieChart.h"

//
class AssetsSummaryView : public QWidget
{
    Q_OBJECT
public:
    explicit AssetsSummaryView(QWidget *parent = nullptr);

private:
    QWidget*                        buildTitleFrame();
    QWidget*                        buildTreeView();
    void                            updateUI();

signals:

public slots:
    void                            onUpdate();
    void                            onQuoteAssetChanged();
    void                            onCurrentRowChanged(QModelIndex index,QModelIndex index2);
    void                            onWalletsChanged();
    void                            onAccountChanged();

private:
    typedef QList<QStandardItem*>   Row;
    QLabel*                         _labelTotalBalance;
    QLabel*                         _labelTotalBalanceFiat;
    QLabel*                         _labelAvailableBalance;
    QLabel*                         _labelAvailableBalanceFiat;
    QTreeView*                      _treeView;
    GenericSortFilterProxyModel*    _sortFilter;
    QStandardItemModel*             _model;
    QIcon                           _avatarIcon;
    bool                            _upToDate           =false;
    QMap<QString,Row>               _rows;
    PieChart*                       _assetsPie          =nullptr;
    PieChart*                       _managerPie         =nullptr;
};

#endif // ASSETSSUMMARYVIEW_H
