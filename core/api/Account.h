#pragma once
#include "../Core.h"
#include "Contact.h"
#include "../rest/RequestResult.h"
#include <QObject>
#include <QJsonObject>

namespace tmio{namespace rest{
class Connection;
}}

TM_CORE_BEGIN(api)

class Account : public QObject
{
    Q_OBJECT
public:
    Account(tmio::rest::Connection* connection,QJsonObject json);
    virtual ~Account();

    Contact*                contact();
    QString                 id();
    QString                 key();
    void                    loadAvatar();
    void                    loadProperties();
    QVariant                accountProperty(const QString& key);
    void                    setAccountProperty(const QString& key,bool value);
    QString                 username();
    bool                    changeUsername(QString newDisplayName);
    QString                 displayName();
    bool                    changeDisplayName(QString newDisplayName);
    QByteArray              avatarImage();
    bool                    changeAvatarImage(QByteArray newAvatarData);
    bool                    isMemberOf(const QString& group);
    bool                    removeGroup(const QString& group);
    QStringList             groups();
    QString                 donateAddress(const QString& assetID);

    public slots:
    void                    onSetPropertyResult(rest::RequestResult&);

    signals:
    void                    changed();

    private:
    Contact*                _contact        =nullptr;
    tmio::rest::Connection* _connection     =nullptr;
    QJsonObject             _json;
    QByteArray              _avatarImage;
    QString                 _id;
    QString                 _key;
    QString                 _username;
    QString                 _displayName;
    QStringList             _groups;
    QMap<QString,QVariant>  _properties;
};

TM_CORE_END()
