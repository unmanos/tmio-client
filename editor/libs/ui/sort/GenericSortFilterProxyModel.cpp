#include "GenericSortFilterProxyModel.h"

//
GenericSortFilterProxyModel::GenericSortFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

//
bool GenericSortFilterProxyModel::filterAcceptsRow(int /*sourceRow*/, const QModelIndex &/*sourceParent*/) const
{
    return(true);
}

//
bool GenericSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    {
        QVariant leftData = sourceModel()->data(left,USER_ROLE_SORT_NUMERIC);
        if(!leftData.isNull())
        {
            QVariant rightData = sourceModel()->data(right,USER_ROLE_SORT_NUMERIC);
            if(!rightData.isNull())
            {
                double left=leftData.toDouble();
                double right=rightData.toDouble();
                return(left<right);
            }
        }
    }
    return(QSortFilterProxyModel::lessThan(left, right));
}
