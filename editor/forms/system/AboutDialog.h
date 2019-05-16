#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#include <QDialog>
#include <rest/RequestResult.h>

using namespace tmio;

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    void                buildDonateFrame();
    void                buildLicenseFrame();

public slots:
    void                onReceiveLicense();
    void                onUpdate();
    void                onResetNetStats();
    void                onV2TestRequest();
    void                onReceiveV2TestRequest(rest::RequestResult& result);

private:
    Ui::AboutDialog*    ui;
    tmio::rest::Request*_v2TestRequest          =nullptr;
};

#endif // ABOUTDIALOG_H
