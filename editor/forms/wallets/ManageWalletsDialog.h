#ifndef MANAGEWALLETSDIALOG_H
#define MANAGEWALLETSDIALOG_H

#include <QDialog>

namespace Ui {
class ManageWalletsDialog;
}

class ManageWalletsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManageWalletsDialog(QWidget *parent = nullptr);
    ~ManageWalletsDialog();

private:
    Ui::ManageWalletsDialog *ui;
};

#endif // MANAGEWALLETSDIALOG_H
