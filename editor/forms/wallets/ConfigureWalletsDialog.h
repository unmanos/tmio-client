#ifndef CONFIGUREWALLETSDIALOG_H
#define CONFIGUREWALLETSDIALOG_H
#include <QDialog>
#include <QMap>
#include <QJsonObject>

class QListWidgetItem;
class ConfigureWalletWidget;

namespace Ui {
class ConfigureWalletsDialog;
}

class ConfigureWalletsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureWalletsDialog(QWidget *parent = nullptr);
    ~ConfigureWalletsDialog();

private:
    QListWidgetItem*            itemFromWalletId(int id);
    void                        refreshWalletList(int selected);
    virtual void                accept();

public slots:
    void                        onAddWallet();
    void                        onRemoveWallet();
    void                        onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*);
    void                        onWalletConfigChanged(int id,QJsonObject config);

private:
    Ui::ConfigureWalletsDialog* ui;
    QMap<int,QJsonObject>       _wallets;
    ConfigureWalletWidget*      _currentWalletWidget    =nullptr;
};

#endif // CONFIGUREWALLETSDIALOG_H
