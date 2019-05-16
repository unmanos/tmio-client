#ifndef TMIOCORE_MANAGEDWALLET_COINBASE_H
#define TMIOCORE_MANAGEDWALLET_COINBASE_H
#include <QObject>
#include "ManagedWallet.h"

//
class QNetworkRequest;

//
TM_CORE_BEGIN(am)

//
class ManagedWallet_Coinbase : public ManagedWallet
{
public:
    ManagedWallet_Coinbase(AssetsManager* assetsManager,QJsonObject config);

private:
    QByteArray              getHMAC(const QString & message);
    void                    getAccount();

public slots:
    void                    onUpdate();
    void                    onReceiveAccount();

private:
    QString                 _apiKey;
    QString                 _apiSecret;
};

TM_CORE_END()
#endif // TMIOCORE_MANAGEDWALLET_COINBASE_H
