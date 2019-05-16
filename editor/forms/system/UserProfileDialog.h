#ifndef USERPROFILEDIALOG_H
#define USERPROFILEDIALOG_H
#include <QDialog>
#include <rest/Request.h>

namespace Ui {
class UserProfileDialog;
}

class UserProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserProfileDialog(QWidget *parent = 0);
    ~UserProfileDialog();

    virtual void                    accept();

private:
    bool                            save();

public slots:
    void                            onBrowseAvatar();
    void                            onSave();
    void                            onUsernameChanged(const QString& text);
    void                            onReceiveUsernameAvailability(tmio::rest::RequestResult&);

private:
    tmio::rest::Request*            _usernameAvailabilityRequest     =nullptr;
    Ui::UserProfileDialog*          ui;
    QByteArray                      _newAvatarData;
};

#endif // USERPROFILEDIALOG_H
