#include "Settings.h"
#include <Core.h>
#include <QDataStream>
#include <aes/QAESEncryption.h>

//
#define TAG                             "Settings"
#define ENCRYPTED_VALUE_MAGIC_UUID      "{fe203e6f-af1f-401a-918a-2d563962ca17}"

//
Settings::Settings()
{
    _delegate=new QSettings(APP_ORGANIZATION, APP_NAME);
}

//
void                        Settings::sync()
{
    _delegate->sync();
}

//
void                        Settings::setValue(QString key,QVariant value)
{
    _delegate->setValue(key,value);
}

//
QVariant                    Settings::value(QString key,QVariant defaultValue)
{
    return(_delegate->value(key,defaultValue));
}

//
void                        Settings::setAccountID(QString id,QString key)
{
    _accountID  =id;
    _accountKey =key;
}

//
void                        Settings::setEncryptedValue(QString key,const QByteArray& value)
{
    // TODO: convert value to base64, encrypt it with AES key
    TM_TRACE(TAG,QString("Changed secure value %1 attached to account %2").arg(key).arg(_accountID));

    QByteArray              data;
    QDataStream             stream(&data,QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_8);
    // write data to stream
    stream << QUuid(ENCRYPTED_VALUE_MAGIC_UUID);
    stream << value;
    if(value.size()%16)
    {
        // AES padding
        for(int padding=0;padding<(16-(value.size()%16));padding++)
        {
            stream << static_cast<char>(0);
        }
    }
    // encrypt
    QAESEncryption      aes(QAESEncryption::AES_128,QAESEncryption::ECB,QAESEncryption::ZERO);
    const QByteArray    encryptedValue=aes.encode(data,QByteArray::fromHex(_accountKey.toUtf8()));
    // write
    _delegate->setValue("encrypted."+_accountID+"."+key,
                        QString::fromStdString(encryptedValue.toBase64().toStdString()));
}

//
QByteArray                  Settings::encryptedValue(QString key)
{
    QString                 encryptedValue  =_delegate->value("encrypted."+_accountID+"."+key).toByteArray();
    if(encryptedValue.length())
    {
        QAESEncryption          aes(QAESEncryption::AES_128,QAESEncryption::ECB,QAESEncryption::ZERO);
        QByteArray              decryptedValue=aes.decode(QByteArray::fromBase64(encryptedValue.toUtf8()),QByteArray::fromHex(_accountKey.toUtf8()));
        QDataStream             stream(&decryptedValue,QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_4_8);
        QUuid                   magic;
        stream >> magic;
        Q_ASSERT(magic==QUuid(ENCRYPTED_VALUE_MAGIC_UUID));
        if(magic==QUuid(ENCRYPTED_VALUE_MAGIC_UUID))
        {
            QByteArray          data;
            stream >> data;
            return(data);
        }
    }
    return(QByteArray());
}

//
void                        Settings::updateAssetsManager(tmio::am::AssetsManager* assetsManager)
{
    assetsManager->reset();
    const QByteArray    savedWallets=encryptedValue(SETTINGS_WALLETS_CONFIGS);
    QJsonDocument       savedDoc=QJsonDocument::fromJson(savedWallets);
    if(savedDoc.isArray())
    {
        for(int i=0;i<savedDoc.array().count();i++)
        {
            QJsonObject         jso=savedDoc.array().at(i).toObject();
            int                 id=jso["id"].toInt();
            if(id)
            {
                assetsManager->addWallet(jso);
            }
        }
    }
}
