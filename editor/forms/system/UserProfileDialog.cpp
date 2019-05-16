#include <QFileDialog>
#include <QBuffer>
#include <QMovie>
#include <QRegExp>

//
#include "Application.h"
#include "UserProfileDialog.h"
#include "ui_UserProfileDialog.h"

//
#define TAG "UserProfileDialog"

//
#define API_CHECK_USERNAME_AVAILABILITY             "accounts/current/username"

//
#define USERNAME_CHECK_ICON_SIZE                    16

//
UserProfileDialog::UserProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserProfileDialog)
{
    ui->setupUi(this);
    //
    rest::Connection&       conn=Application::instance().connection();
    if(conn.account()->avatarImage().length())
    {
        QImage image;
        //image.load("/home/pierre/mur en pierre.JPG");
        image.loadFromData(conn.account()->avatarImage());
        QPixmap buffer = QPixmap::fromImage(image);
        ui->avatarImage->setPixmap(buffer);
                    //),this,SLOT(onBrowseAvatar));
    }
    else
    {
        ui->avatarImage->setPixmap(QPixmap());
        ui->avatarImage->setText(tr("(No avatar)"));
    }
    connect(ui->browseAvatarButton,SIGNAL(clicked(bool)),this,SLOT(onBrowseAvatar()));
    ui->displayName->setText(conn.account()->displayName());
    ui->username->setText(conn.account()->username());
    ui->usernameAvailabilityIcon->hide();
    connect(ui->username,SIGNAL(textChanged(const QString&)),this,SLOT(onUsernameChanged(const QString&)));
    // groups
    ui->groups->setEnabled(false);
    conn.account()->groups();
    if(conn.account()->groups().length())
    {
        QString str;
        foreach(QString g, conn.account()->groups())
        {
            str+=str.isEmpty()?g:(", "+g);
        }
        ui->groups->setText(str);
    }
    else
    {
        ui->groups->setText("(none)");
    }
    // wait
    ui->wait->hide();
    QMovie *movie = new QMovie(":/loading/spin.gif");
    movie->setParent(this);
    movie->setScaledSize(QSize(64,64));
    ui->wait->setMovie(movie);
    ui->wait->setAlignment(Qt::AlignCenter);
    movie->start();
    // connect signal
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(onSave()));
}

//
UserProfileDialog::~UserProfileDialog()
{
    delete ui;
}

//
void                            UserProfileDialog::accept()
{
    tmio::rest::Connection&   conn      =Application::instance().connection();
    ui->frame->hide();
    ui->wait->show();
    ui->buttonBox->setEnabled(false);
    bool                      succeeded =true;
    // save data
    if(conn.account()->username()!=ui->username->text())
    {
        if(!Application::instance().connection().account()->changeUsername(ui->username->text()))
        {
            succeeded=false;
        }
    }
    if(succeeded)
    {
        if(conn.account()->displayName()!=ui->displayName->text())
        {
            Application::instance().connection().account()->changeDisplayName(ui->displayName->text());
        }
    }
    if(succeeded)
    {
        if(_newAvatarData.length())
        {
            Application::instance().connection().account()->changeAvatarImage(_newAvatarData);
        }
    }
    // close dialog if succeeded
    ui->frame->show();
    ui->wait->hide();
    ui->buttonBox->setEnabled(true);
    if(succeeded)
    {
        QDialog::accept();
    }
}

//
bool                            UserProfileDialog::save()
{
    return(true);
}

//
void                            UserProfileDialog::onBrowseAvatar()
{
    TM_TRACE(TAG,"onBrowseAvatar");
    QString     fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if(fileName.length())
    {
        TM_TRACE(TAG,"Selected file "+fileName);
        ui->avatarImage->setPixmap(QPixmap());
        QImage image;
        if(image.load(fileName))
        {
            // TODO: Rotate image with QImageReader
            QImage avatar=image.scaled(128,128,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            QPixmap pixmap = QPixmap::fromImage(avatar);
            //avatar.save()
            //QByteArray byteArray;
            QBuffer buffer;
            if(avatar.save(&buffer,"PNG"))
            {
                TM_TRACE(TAG,QString("Avatar size %1").arg(buffer.buffer().length()));
                ui->avatarImage->setPixmap(pixmap);
                _newAvatarData=buffer.buffer();
            }
            else
            {
                TM_ERROR(TAG,"Image convertion failed");
                ui->avatarImage->setText(tr("(Wrong image format)"));
            }
        }
        else
        {
            TM_ERROR(TAG,"Load image failed");
            ui->avatarImage->setText(tr("(Load image failed)"));
        }
    }
}

//
void                        UserProfileDialog::onSave()
{
    TM_TRACE(TAG,"onSave");
    if(save())
    {
        TM_TRACE(TAG,"Saved");
        //accept();
    }
    else
    {
        TM_ERROR(TAG,"Save failed");
    }
}

//
void                        UserProfileDialog::onUsernameChanged(const QString& text)
{
    TM_TRACE(TAG,"onUsernameChanged");
    if(_usernameAvailabilityRequest)
    {
        delete(_usernameAvailabilityRequest);
        _usernameAvailabilityRequest=nullptr;
    }

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    QRegExp         rx("^(\\w){4,16}$");
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPatternSyntax(QRegExp::RegExp);
    if(rx.exactMatch(text))
    {
        ui->usernameAvailabilityIcon->show();
        ui->usernameAvailabilityIcon->setPixmap(QIcon(":/icons/common/hourglass.png").pixmap(USERNAME_CHECK_ICON_SIZE,USERNAME_CHECK_ICON_SIZE));
        ui->usernameAvailability->setStyleSheet("");
        ui->usernameAvailability->setText("Checking availability...");
        _usernameAvailabilityRequest=Application::instance().connection().createRequest(QString(API_CHECK_USERNAME_AVAILABILITY));
        connect(_usernameAvailabilityRequest, SIGNAL(finished(tmio::rest::RequestResult&)), this, SLOT(onReceiveUsernameAvailability(tmio::rest::RequestResult&)));
        QJsonObject         postData;
        postData.insert("name",text);
        postData.insert("checkAvailability",true);
        _usernameAvailabilityRequest->post(postData,false);
    }
    else
    {
        ui->usernameAvailabilityIcon->show();
        ui->usernameAvailabilityIcon->setPixmap(QIcon(":/icons/common/cross.png").pixmap(USERNAME_CHECK_ICON_SIZE,USERNAME_CHECK_ICON_SIZE));
        ui->usernameAvailability->setStyleSheet("QLabel { color : red; }");
        ui->usernameAvailability->setText("Invalid username (must not contains special characters, 4-16 length)");
    }
}

//
void                        UserProfileDialog::onReceiveUsernameAvailability(tmio::rest::RequestResult& result)
{
    if(result._statusCode==200)
    {
        TM_TRACE(TAG,"Username available");
        ui->usernameAvailabilityIcon->show();
        ui->usernameAvailabilityIcon->setPixmap(QIcon(":/icons/common/check.png").pixmap(USERNAME_CHECK_ICON_SIZE,USERNAME_CHECK_ICON_SIZE));
        ui->usernameAvailability->setStyleSheet("QLabel { color : green; }");
        ui->usernameAvailability->setText("Username available");
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    }
    else
    {
        TM_WARNING(TAG,"Username NOT available");
        ui->usernameAvailabilityIcon->show();
        ui->usernameAvailabilityIcon->setPixmap(QIcon(":/icons/common/cross.png").pixmap(USERNAME_CHECK_ICON_SIZE,USERNAME_CHECK_ICON_SIZE));
        ui->usernameAvailability->setStyleSheet("QLabel { color : red; }");
        ui->usernameAvailability->setText("Username not available");
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    }
    delete(_usernameAvailabilityRequest);
    _usernameAvailabilityRequest=nullptr;
}
