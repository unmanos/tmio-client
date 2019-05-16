#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>

#include <rest/Connection.h>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void openSession();
    void resetUI();
    void buildUI_opensessionfailed(tmio::rest::Connection::AuthResult);
    void buildUI_login(const QString& message="");
    void buildUI_register();
    void buildUI_recover();
    void buildUI_captcha(bool retry,const QString errorText);
    void buildUI_checkmail(const QString& message);
    bool validate();
    void login();
    void registerAccount();
    void recoverAccount();
    void submitCaptcha();
    void submitMailCode();
    void clearErrors();
    void setBusy(bool busy);

private slots:
    void onLanguageChanged();
    void onStyleChanged();
    void onOK();
    void onLinkActivated(const QString&);
    void onCaptchaAnswerEdited(const QString &);

private:
    Ui::LoginDialog*    ui;
    QString             _mode;
    QString             _createAccountEmail;
    QString             _recoverAccountEmail;
    QLineEdit*          _email                  =nullptr;
    QLabel*             _emailError             =nullptr;
    QLabel*             _passwordError          =nullptr;
    QLabel*             _confirmPasswordError   =nullptr;
    QLabel*             _error                  =nullptr;
    QLineEdit*          _password               =nullptr;
    QLineEdit*          _confirmPassword        =nullptr;
    QLabel*             _labelCaptcha           =nullptr;
    QLineEdit*          _captchaAnswer          =nullptr;
    QLabel*             _captchaAnswerError     =nullptr;
    QCheckBox*          _saveCredentials        =nullptr;
    QString             _afterCaptchaMode;
};

#endif // LOGINDIALOG_H
