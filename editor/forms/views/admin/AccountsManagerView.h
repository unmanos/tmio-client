#ifndef ACCOUNTSMANAGERVIEW_H
#define ACCOUNTSMANAGERVIEW_H
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <rest/RequestResult.h>

//
class AccountsManagerView : public QWidget
{
    Q_OBJECT
public:
    explicit AccountsManagerView(QWidget *parent = nullptr);

    void                            updateAccounts();

signals:

public slots:
    void                            onReceiveAccounts(tmio::rest::RequestResult&);

private:
    tmio::rest::Request*            _accountsRequest    =nullptr;
    QTreeView*                      _treeView           =nullptr;
    QStandardItemModel*             _model              =nullptr;
};

#endif // ACCOUNTSMANAGERVIEW_H
