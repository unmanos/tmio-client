#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>

namespace Ui {
class LoadingDialog;
}

class LoadingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingDialog(QWidget *parent = 0);
    ~LoadingDialog();

    void                nextPhase();

public slots:
    void                onCheckReady();

private:
    Ui::LoadingDialog *ui;
    int                 _phase=0;
};

#endif // LOADINGDIALOG_H
