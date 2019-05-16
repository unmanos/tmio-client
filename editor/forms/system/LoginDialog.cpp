#include <QTimer>
#include "LoginDialog.h"
#include "config.h"
#include "ui_LoginDialog.h"
#include "../../Application.h"
#include <QTranslator>
#include <QLibraryInfo>
#include <QMovie>
#include <QAbstractButton>
#include <QFormLayout>
#include <QMessageBox>

#include <aes/QAESEncryption.h>

//
#define TAG             "LoginDialog"

#define AES_START_KEY   "ac66723ecfd545b18f5a89eaf18ced26"

#ifdef QT_DEBUG
#define DEBUG_LOGIN_FIELDS
#endif

//
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    Settings&       settings=Application::instance().settings();
    if(settings.value(SETTINGS_LOGIN+"/style","light")=="dark")
    {
        Application::instance().style().set(Style::styleDark);
    }

    ui->setupUi(this);

    ui->language->addItem("English");
    ui->language->setItemIcon(0,QIcon(":/icons/flags/uk.png"));
    /*ui->language->addItem("Deutsch");
    ui->language->setItemIcon(2,QIcon(":/icons/flags/de.png"));*/
    ui->language->hide();

    ui->style->addItem("Light mode",QString("light"));
    ui->style->addItem("Dark mode (experimental)",QString("dark"));
    if(settings.value(SETTINGS_LOGIN+"/style","light")=="dark")
    {
        ui->style->setCurrentIndex(1);
    }
    ui->style->hide();

    connect(ui->style,SIGNAL(currentIndexChanged(int)),this,SLOT(onStyleChanged()));
    connect(ui->language,SIGNAL(currentIndexChanged(int)),this,SLOT(onLanguageChanged()));

    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(onOK()));

    // create session
    openSession();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

//
static void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

//
void LoginDialog::openSession()
{
    setBusy(true);
    rest::Connection&   connection  =Application::instance().connection();
    Settings&           settings    =Application::instance().settings();
    const QString       token       =settings.value(SETTINGS_API_SESSION_TOKEN,"").toString();
    const tmio::rest::Connection::AuthResult
                        authResult  =connection.openSession(TMEDITOR_API_SERVER,"tokenmark.io/desktop",token);
    if(authResult==tmio::rest::Connection::AuthResult::Succeeded)
    {
        settings.setValue(SETTINGS_API_SESSION_TOKEN,connection.token());
        if(connection.isHuman())
        {
            buildUI_login();
        }
        else
        {
            buildUI_captcha(false,"");
        }
    }
    else
    {
        buildUI_opensessionfailed(authResult);
    }
    setBusy(false);
}

//
void LoginDialog::resetUI()
{
    clearLayout(ui->fieldsFrame, true);
    _email                  =nullptr;
    _emailError             =nullptr;
    _passwordError          =nullptr;
    _confirmPasswordError   =nullptr;
    _error                  =nullptr;
    _password               =nullptr;
    _confirmPassword        =nullptr;
    _saveCredentials        =nullptr;
    _labelCaptcha           =nullptr;
    _captchaAnswer          =nullptr;
    _captchaAnswerError     =nullptr;
}

//
void LoginDialog::buildUI_opensessionfailed(tmio::rest::Connection::AuthResult authResult)
{
    _mode="opensessionfailed";
    resetUI();
    QFormLayout*            formLayout=new QFormLayout();
    //formLayout
    ui->fieldsFrame->addLayout(formLayout);
    ui->fieldsFrame->addWidget(_error=new QLabel());
    _error->setStyleSheet("QLabel { color : red; }");
    _error->setWordWrap(true);
    switch(authResult)
    {
        //
        case tmio::rest::Connection::AuthResult::NetworkError:
        case tmio::rest::Connection::AuthResult::SslLibraryError:
        _error->setText("Failed to open session, please check your internet connection and try again.");
        break;
        //
        case tmio::rest::Connection::AuthResult::APIVersionMismatch:
        _error->setText("The client you are using to connect does not support the new API version.\r\nPlease update the application.");
        break;
        //
        default:
        _error->setText("Unable to open session, please contact support.");
        break;
        //
    }

}

//
void LoginDialog::buildUI_login(const QString& message)
{
    resetUI();
    rest::Connection&   connection=Application::instance().connection();
    _mode="login";
    Settings&           settings=Application::instance().settings();

    //ui->fieldsFrame->layout()->clea
    //ui->fieldsFrame->remo
    QFormLayout*            formLayout=new QFormLayout();

    //formLayout
    ui->fieldsFrame->addLayout(formLayout);
    ui->fieldsFrame->addWidget(_error=new QLabel());
    /* API Server combo                 */
    QLabel*                 labelRegister=new QLabel(message.length()?message:"No account yet? <a href='register'>Create one</a>. Lost password? <a href='recover'>Recover</a>.");
    connect(labelRegister,SIGNAL(linkActivated(const QString&)),this,SLOT(onLinkActivated(const QString&)));

    formLayout->addRow(nullptr,labelRegister);
    /* Username                         */
    formLayout->addRow("E-mail",_email=new QLineEdit());
    formLayout->addRow(nullptr,_emailError=new QLabel());
    formLayout->addRow("Password",_password=new QLineEdit());
    formLayout->addRow(nullptr,_passwordError=new QLabel());
    formLayout->addRow(nullptr,_saveCredentials=new QCheckBox("Save credentialson this computer"));
    _password->setEchoMode(QLineEdit::Password);
    _emailError->setStyleSheet("QLabel { color : red; }");
    _passwordError->setStyleSheet("QLabel { color : red; }");
    _error->setStyleSheet("QLabel { color : red; }");
    _error->setWordWrap(true);
    //_emailError->set

    //formLayout->insertRow()
    if(_createAccountEmail.isEmpty())
    {
        QAESEncryption      aes(QAESEncryption::AES_128,QAESEncryption::ECB,QAESEncryption::ZERO);
        const QByteArray    encEmail       =QByteArray::fromBase64(settings.value(SETTINGS_LOGIN+"/email","").toString().toUtf8());
        const QByteArray    encPassword    =QByteArray::fromBase64(settings.value(SETTINGS_LOGIN+"/password","").toString().toUtf8());
        QString             email          =QString::fromUtf8(aes.decode(encEmail,connection.sessionKey()));
        QString             password       =QString::fromUtf8(aes.decode(encPassword,connection.sessionKey()));
        _email->setText(email.startsWith(AES_START_KEY)?email.right(email.length()-32):"");
        if(!_email->text().isEmpty())
        {
            _password->setText(password.startsWith(AES_START_KEY)?password.right(password.length()-32):"");
            _saveCredentials->setChecked(_password->text().length()!=0);
        }
    }
    else
    {
        _email->setText(_createAccountEmail);
    }
    clearErrors();
}

//
void LoginDialog::buildUI_register()
{
    resetUI();
    qDebug() << "Build UI";
    _mode="register";
    //ui->fieldsFrame->layout()->clea
    //ui->fieldsFrame->remo
    QFormLayout*            formLayout=new QFormLayout();

    //formLayout
    ui->fieldsFrame->addLayout(formLayout);
    ui->fieldsFrame->addWidget(_error=new QLabel());
    /* API Server combo                 */
    QLabel*                 labelLogin=new QLabel("Already have an account? <a href='login'>Login.</a>");
    connect(labelLogin,SIGNAL(linkActivated(const QString&)),this,SLOT(onLinkActivated(const QString&)));

    formLayout->addRow(nullptr,labelLogin);
    /* Username                         */
    formLayout->addRow("E-mail",_email=new QLineEdit());
    formLayout->addRow(nullptr,_emailError=new QLabel());
    formLayout->addRow("Password",_password=new QLineEdit());
    formLayout->addRow(nullptr,_passwordError=new QLabel());
    formLayout->addRow("Confirm password",_confirmPassword=new QLineEdit());
    formLayout->addRow(nullptr,_confirmPasswordError=new QLabel());
    _password->setEchoMode(QLineEdit::Password);
    _confirmPassword->setEchoMode(QLineEdit::Password);
    _emailError->setStyleSheet("QLabel { color : red; }");
    _passwordError->setStyleSheet("QLabel { color : red; }");
    _confirmPasswordError->setStyleSheet("QLabel { color : red; }");
    _error->setStyleSheet("QLabel { color : red; }");
    _error->setWordWrap(true);
    clearErrors();
    // debug
    #ifdef DEBUG_LOGIN_FIELDS
    _email->setText("test@tokenmark.io");
    _password->setText("12345678");
    _confirmPassword->setText("12345678");
    #endif
}

//
void LoginDialog::buildUI_recover()
{
    resetUI();
    qDebug() << "Build UI recover";
    _mode="recover";
    QFormLayout*            formLayout=new QFormLayout();
    ui->fieldsFrame->addLayout(formLayout);
    ui->fieldsFrame->addWidget(_error=new QLabel());
    /* API Server combo                 */
    QLabel*                 labelLogin=new QLabel("Enter your account email and your new password.");
    formLayout->addRow(nullptr,labelLogin);
    /* Username                         */
    formLayout->addRow("Account E-mail",_email=new QLineEdit());
    formLayout->addRow(nullptr,_emailError=new QLabel());
    formLayout->addRow("New password",_password=new QLineEdit());
    formLayout->addRow(nullptr,_passwordError=new QLabel());
    formLayout->addRow("Confirm password",_confirmPassword=new QLineEdit());
    formLayout->addRow(nullptr,_confirmPasswordError=new QLabel());
    _password->setEchoMode(QLineEdit::Password);
    _confirmPassword->setEchoMode(QLineEdit::Password);
    _emailError->setStyleSheet("QLabel { color : red; }");
    _passwordError->setStyleSheet("QLabel { color : red; }");
    _confirmPasswordError->setStyleSheet("QLabel { color : red; }");
    _error->setStyleSheet("QLabel { color : red; }");
    _error->setWordWrap(true);
    clearErrors();
    // debug
    #ifdef DEBUG_LOGIN_FIELDS
    _email->setText("test@tokenmark.io");
    _password->setText("12345678");
    _confirmPassword->setText("12345678");
    #endif
}

//
void LoginDialog::buildUI_captcha(bool retry,const QString errorText)
{
    if(!retry)
    {
        resetUI();
        qDebug() << "Build UI";
        _mode="captcha";
        QFormLayout*            formLayout=new QFormLayout();
        ui->fieldsFrame->addLayout(formLayout);
        ui->fieldsFrame->addWidget(_error=new QLabel());
        /* API Server combo                 */
        QLabel*                 labelHelp=new QLabel("You must solve this captcha to continue...");
        formLayout->addRow(nullptr,labelHelp);
        formLayout->addRow("Captcha",_labelCaptcha=new QLabel());
        formLayout->addRow("Answer",_captchaAnswer=new QLineEdit());
        connect(_captchaAnswer, SIGNAL(textEdited(const QString &)), SLOT(onCaptchaAnswerEdited(const QString &)));
        formLayout->addRow(nullptr,_captchaAnswerError=new QLabel());
        _captchaAnswerError->setStyleSheet("QLabel { color : red; }");
        _error->setStyleSheet("QLabel { color : red; }");
        _error->setWordWrap(true);
        clearErrors();
        if(errorText.length())
        {
            _error->setText(errorText);
            _error->show();
        }
    }
    else
    {
        _error->show();
        _error->setText("Failed to solve captcha, try again.");
    }
    rest::Connection&   connection      =Application::instance().connection();
    rest::RequestResult captchaResult   =connection.rGET("session/captcha");
    QImage image;
    image.loadFromData(captchaResult._raw);
    QImage scaledImage=image.scaled(image.width()*1.5,image.height()*1.5,Qt::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation);
    QPixmap buffer = QPixmap::fromImage(scaledImage);
    _labelCaptcha->setPixmap(buffer);
    _captchaAnswer->setText("");
    _captchaAnswer->setFocus();
}

//
void LoginDialog::buildUI_checkmail(const QString& message)
{
    _mode="checkmail";
    resetUI();
    QFormLayout*            formLayout=new QFormLayout();
    QLabel*                 labelHelp=new QLabel(message);
    labelHelp->setWordWrap(true);
    formLayout->addRow(nullptr,labelHelp);
    formLayout->addRow("Answer",_captchaAnswer=new QLineEdit());
    connect(_captchaAnswer, SIGNAL(textEdited(const QString &)), SLOT(onCaptchaAnswerEdited(const QString &)));
    formLayout->addRow(nullptr,_captchaAnswerError=new QLabel());
    _captchaAnswerError->setStyleSheet("QLabel { color : red; }");
    ui->fieldsFrame->addLayout(formLayout);
    ui->fieldsFrame->addWidget(_error=new QLabel());
    _error->setStyleSheet("QLabel { color : red; }");
    _error->setWordWrap(true);
}

//
bool LoginDialog::validate()
{
    qDebug() << "validate";
    bool pass=true;
    clearErrors();
    if(_email)
    {
        QRegExp mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
        mailREX.setCaseSensitivity(Qt::CaseInsensitive);
        mailREX.setPatternSyntax(QRegExp::RegExp);
        if(!mailREX.exactMatch(_email->text()))
        {
            _emailError->setText(tr("Invalid email address"));
            pass=false;
        }

    }
    if(_password)
    {
        if(_password->text().length()<8)
        {
            _passwordError->setText(tr("Password too short"));
            pass=false;
        }
    }
    if(_captchaAnswer)
    {
        if(_captchaAnswer->text().length()<6)
        {
            _captchaAnswerError->setText(tr("Answer too short"));
            pass=false;
        }
    }
    if( (_mode=="register")||(_mode=="recover") )
    {
        if(_password->text()!=_confirmPassword->text())
        {
            _confirmPasswordError->setText(tr("Password does not match"));
            pass=false;
        }
    }
    return(pass);
}

//
void LoginDialog::login()
{
    Settings&       settings=Application::instance().settings();
    if(validate())
    {
        rest::Connection&   connection=Application::instance().connection();
        //ui->wait->show();
        tmio::rest::Connection::AuthResult result=connection.authenticate(_email->text(),_password->text());
        //ui->wait->hide();
        if(result==tmio::rest::Connection::AuthResult::Succeeded)
        {
            TM_TRACE(TAG,"Succeeded, starting main window");
            bool saveCredentials=_saveCredentials->checkState()==Qt::CheckState::Checked;
            QAESEncryption      aes(QAESEncryption::AES_128,QAESEncryption::ECB,QAESEncryption::ZERO);
            const QByteArray    email       =QString(AES_START_KEY+QString(saveCredentials?_email->text():"")).toUtf8();
            const QByteArray    password    =QString(AES_START_KEY+QString(saveCredentials?_password->text():"")).toUtf8();
            const QByteArray    encEmail    =aes.encode(email,connection.sessionKey());
            const QByteArray    encPassword =aes.encode(password,connection.sessionKey());
            settings.setValue(SETTINGS_LOGIN+"/email",QString::fromStdString(encEmail.toBase64().toStdString()));
            settings.setValue(SETTINGS_LOGIN+"/password",QString::fromStdString(encPassword.toBase64().toStdString()));
            settings.sync();
            // ok
            accept();
            Application::instance().attachAccount(Application::instance().connection().account()->id(),
                                                  Application::instance().connection().account()->key());
            Application::instance().style().set((ui->style->currentIndex()==0)?Style::styleLight:Style::styleDark);
        }
        else
        {
            if(connection.isHuman())
            {
                switch(result)
                {
                    case tmio::rest::Connection::AuthResult::NetworkError:
                    _error->setText(tr("Unable to contact authentification server, please check you are connected to Internet."));
                    break;
                    case tmio::rest::Connection::AuthResult::SslLibraryError:
                    _error->setText(tr("Unable to find SSL libraries, please check OpenSSL is correctly installed and try again."));
                    break;
                    case tmio::rest::Connection::AuthResult::AuthFailed:
                    _error->setText(tr("Authentication failed, please check your email and password."));
                    Application::instance().settings().setValue(SETTINGS_LOGIN+"/password","");
                    _password->setText("");
                    break;
                    case tmio::rest::Connection::AuthResult::LoadAccountFailed:
                    _error->setText(tr("Unable to access your account, please contact support team."));
                    break;
                    default:
                    _error->setText(tr("Unknown error."));
                    break;
                }
                _error->show();
            }
            else
            {
                _afterCaptchaMode="login";
                buildUI_captcha(false,tr("You failed to authenticate, please proove your humanity by solving this captcha."));
            }
        }
        TM_TRACE(TAG, "OK finished the loop");
    }
}

//
void LoginDialog::registerAccount()
{
    if(validate())
    {
        const QString                   email       =_email->text();
        const QString                   password    =_password->text();
        rest::Connection&               connection  =Application::instance().connection();
        rest::Connection::AuthResult    result      =connection.createAccount(email,password);
        if(result==tmio::rest::Connection::AuthResult::Succeeded)
        {
            _createAccountEmail=email;
            buildUI_checkmail("We sent you an email to confirm your account, please enter the code below...");
        }
        else
        {
            if(connection.isHuman())
            {
                switch(result)
                {
                    case tmio::rest::Connection::AuthResult::NetworkError:
                    _error->setText(tr("Unable to contact authentification server, please check you are connected to Internet."));
                    break;
                    case tmio::rest::Connection::AuthResult::SslLibraryError:
                    _error->setText(tr("Unable to find SSL libraries, please check OpenSSL is correctly installed and try again."));
                    break;
                    case tmio::rest::Connection::AuthResult::CreateAccountFailed:
                    _error->setText(tr("Unable to create account, only one account per mail account is permitted."));
                    break;
                    case tmio::rest::Connection::AuthResult::TooManyAccounts:
                    _error->setText(tr("Unable to create account. The account creation is temporarily paused, please come back tomorrow."));
                    break;
                    default:
                    _error->setText(tr("Unknown error."));
                    break;
                }
                _error->show();
            }
            else
            {
                //buildUI_captcha(false,tr("You failed too many times to authenticate, please proove your humanity by solving this captcha."));
                _afterCaptchaMode="register";
                buildUI_captcha(false,tr("Unable to create account, please proove your humanity by solving this captcha."));
            }
        }
        TM_TRACE(TAG, "OK finished the loop");
    }
}

//
void LoginDialog::recoverAccount()
{
    if(validate())
    {
        const QString                   email       =_email->text();
        const QString                   password    =_password->text();
        rest::Connection&               connection  =Application::instance().connection();
        rest::Connection::AuthResult    result      =connection.recoverAccount(email,password);
        if(result==tmio::rest::Connection::AuthResult::Succeeded)
        {
            _recoverAccountEmail=email;
            buildUI_checkmail("We sent you an email to finish the password change procedure, please enter the code below...");
        }
        else
        {
            if(connection.isHuman())
            {
                switch(result)
                {
                    case tmio::rest::Connection::AuthResult::NetworkError:
                    _error->setText(tr("Unable to contact authentification server, please check you are connected to Internet."));
                    break;
                    case tmio::rest::Connection::AuthResult::SslLibraryError:
                    _error->setText(tr("Unable to find SSL libraries, please check OpenSSL is correctly installed and try again."));
                    break;
                    case tmio::rest::Connection::AuthResult::RecoverAccountFailed:
                    _error->setText(tr("Unable to recover account, please contact support."));
                    break;
                    default:
                    _error->setText(tr("Unknown error."));
                    break;
                }
                _error->show();
            }
            else
            {
                _afterCaptchaMode="recover";
                buildUI_captcha(false,tr("Unable to recover, please proove your humanity by solving this captcha."));
            }
        }
        TM_TRACE(TAG, "OK finished the loop");
    }
}

//
void LoginDialog::submitCaptcha()
{
    if(validate())
    {
        rest::Connection&   connection=Application::instance().connection();
        //ui->wait->show();

        QJsonObject     answer;
        answer.insert("answer",_captchaAnswer->text());
        tmio::rest::RequestResult result=connection.rPOST("session/captcha",answer);
        if(result._statusCode!=200)
        {
            buildUI_captcha(true,"");
        }
        else
        {
            // ok
            if(_afterCaptchaMode=="register")
            {
                buildUI_register();
            }
            else if(_afterCaptchaMode=="recover")
            {
                buildUI_recover();
            }
            else
            {
                buildUI_login();
            }
        }
    }
}

//
void LoginDialog::submitMailCode()
{
    if(validate())
    {
        rest::Connection&   connection=Application::instance().connection();
        //ui->wait->show();

        QJsonObject     answer;
        answer.insert("answer",_captchaAnswer->text());
        tmio::rest::RequestResult result=connection.rPOST("session/mailcode",answer);
        if(result._statusCode!=200)
        {
            //buildUI_confirmok
            _error->setText("Invalid mail code, please check your mailbox.");
            _error->show();
            _captchaAnswer->setText("");
        }
        else
        {
            QString         text;
            if(connection.isHuman())
            {
                if(!_createAccountEmail.isEmpty())
                    text=tr("Account created successfully, please login now.");
                if(!_recoverAccountEmail.isEmpty())
                    text=tr("Password changed successfully, please login now.");
                buildUI_login(text);
            }
            else
            {
                if(!_createAccountEmail.isEmpty())
                    text=tr("Account created successfully, please proove your humanity before login.");
                if(!_recoverAccountEmail.isEmpty())
                    text=tr("Password changed successfully, please proove your humanity before login.");
                buildUI_captcha(false,text);
            }
            _createAccountEmail.clear();
            _recoverAccountEmail.clear();
        }
    }
}

//
void LoginDialog::clearErrors()
{
    if(_emailError)
    {
        _emailError->clear();
    }
    if(_passwordError)
    {
        _passwordError->clear();
    }
    if(_confirmPasswordError)
    {
        _confirmPasswordError->clear();
    }
    if(_captchaAnswerError)
    {
        _captchaAnswerError->clear();
    }
    if(_error)
    {
        _error->hide();
    }
}

//
void LoginDialog::setBusy(bool busy)
{
    ui->inputFrame->setEnabled(!busy);
    if(!busy)
    {
        if(_mode=="login")
        {
            if(_email->text().isEmpty())
            {
                _email->setFocus();
            }
            else if(_password->text().isEmpty())
            {
                _password->setFocus();
            }
        }
        if( (_mode=="register")||(_mode=="recover") )
        {
            _email->setFocus();
        }
        if( (_mode=="captcha")||(_mode=="checkmail") )
        {
            _captchaAnswer->setFocus();
        }
    }
}

//
void LoginDialog::onLanguageChanged()
{
    int lang=ui->language->currentIndex();
    switch(lang)
    {
        case 0:
        Application::instance().setTranslation("en");
        break;
        case 1:
        Application::instance().setTranslation("de");
        break;
    }
    clearErrors();
    ui->retranslateUi(this);
}

//
void LoginDialog::onStyleChanged()
{
    Settings&       settings=Application::instance().settings();
    settings.setValue(SETTINGS_LOGIN+"/style",ui->style->currentData().toString());
}

void LoginDialog::onOK()
{
    if(_mode=="login")
    {
        setBusy(true);
        login();
        setBusy(false);
        return;
    }
    if(_mode=="register")
    {
        setBusy(true);
        registerAccount();
        setBusy(false);
        return;
    }
    if(_mode=="recover")
    {
        setBusy(true);
        recoverAccount();
        setBusy(false);
        return;
    }
    if(_mode=="captcha")
    {
        setBusy(true);
        submitCaptcha();
        setBusy(false);
        return;
    }
    if(_mode=="checkmail")
    {
        setBusy(true);
        submitMailCode();
        setBusy(false);
        return;
    }
    if(_mode=="opensessionfailed")
    {
        openSession();
        return;
    }

}


//
void LoginDialog::onLinkActivated(const QString& link)
{
    if(link=="register")
        buildUI_register();
    if(link=="login")
        buildUI_login();
    if(link=="recover")
        buildUI_recover();
}

//
void LoginDialog::onCaptchaAnswerEdited(const QString & /*text*/)
{
    QString     str=_captchaAnswer->text().toUpper();
    str=str.remove(QRegExp("[^a-zA-Z\\d\\s]"));
    _captchaAnswer->setText(str);
}
