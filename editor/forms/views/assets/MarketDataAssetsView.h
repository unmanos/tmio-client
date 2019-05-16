#ifndef MARKETDATAASSETSVIEW_H
#define MARKETDATAASSETSVIEW_H
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMap>
#include <QLabel>
#include "libs/ui/sort/GenericSortFilterProxyModel.h"

//
class QToolButton;
class QComboBox;

//
class MarketDataAssetsView : public QWidget
{
    Q_OBJECT
public:
    explicit MarketDataAssetsView(QWidget *parent = nullptr);

private:
    QString                         friendlyTime(const QDateTime& t);
    void                            updateData();
    bool                            loadSources();

signals:

public slots:
    void                            onFilterTypeChanged(int index);
    void                            onUpdateData();
    void                            onQuoteAssetChanged();
    void                            onCurrentRowChanged(QModelIndex,QModelIndex);
    void                            onSelectionChanged(const QItemSelection&, const QItemSelection&);
    void                            onDoubleClickedAsset(QModelIndex);
    void                            onCreateAsset(bool);
    void                            onDeleteAsset(bool);
    void                            onAssetModified(const QString& id);
    void                            onAssetDeleted(const QString& id);

private:
    typedef QVector<QStandardItem*> Row;
    QTreeView*                      _treeView           =nullptr;
    QStandardItemModel*             _model              =nullptr;
    QMap<QString,Row>               _lookup;
    GenericSortFilterProxyModel*    _sortFilter         =nullptr;
    QToolButton*                    _createButton       =nullptr;
    QToolButton*                    _deleteButton       =nullptr;
    QLabel*                         _labelMarketInfo    =nullptr;
    QLabel*                         _labelMarketCap     =nullptr;
    QLabel*                         _labelBtcDominance  =nullptr;
    QComboBox*                      _filterTypeCombo    =nullptr;
};

#endif // MARKETDATAASSETSVIEW_H
