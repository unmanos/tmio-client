#ifndef SETTINGS_H
#define SETTINGS_H
#include <QSettings>
#include <QUuid>
#include "am/AssetsManager.h"

//
#define APP_ORGANIZATION                "tokenmark.io"
#define APP_NAME                        "tmio-desktop"

//
#define SETTINGS_QUOTE_ASSET_ID         "preferences/default_quote_id"

// nicehash
#define SETTINGS_NICEHASH_API_ID        "exchanges/nicehash/api_id"
#define SETTINGS_NICEHASH_API_KEY       "exchanges/nicehash/api_key"

// wallets
#define SETTINGS_WALLETS_CONFIGS        "wallets/data"

#define SETTINGS_API_SESSION_TOKEN      "api/session/token"

#define SETTINGS_LOGIN                  QString("login")


//
class Settings
{
public:
    Settings();

public:
    enum eType
    {
        tLocal,
        tRemote,
        tSecure
    };

    void                        sync();
    void                        setValue(QString key,QVariant value);
    QVariant                    value(QString key,QVariant defaultValue);
    //
    void                        setAccountID(QString id,QString key);
    void                        setEncryptedValue(QString key,const QByteArray& value);
    QByteArray                  encryptedValue(QString key);
    // assets
    void                        updateAssetsManager(tmio::am::AssetsManager*);


private:
    QSettings*                  _delegate;
    QString                     _accountID;
    QString                     _accountKey;
};

#endif // SETTINGS_H
