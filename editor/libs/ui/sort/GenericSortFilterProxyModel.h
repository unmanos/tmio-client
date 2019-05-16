#ifndef GENERICSORTFILTERPROXYMODEL_H
#define GENERICSORTFILTERPROXYMODEL_H
#include <QSortFilterProxyModel>

//
#define USER_ROLE_SORT_NUMERIC         (Qt::UserRole+100)

//
class GenericSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    GenericSortFilterProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // GENERICSORTFILTERPROXYMODEL_H
