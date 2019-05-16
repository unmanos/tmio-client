#include "ManageWalletsDialog.h"
#include "ui_ManageWalletsDialog.h"

ManageWalletsDialog::ManageWalletsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageWalletsDialog)
{
    ui->setupUi(this);
}

ManageWalletsDialog::~ManageWalletsDialog()
{
    delete ui;
}
