#ifndef TMIOCORE_MANAGEDWALLET_BINANCE_H
#define TMIOCORE_MANAGEDWALLET_BINANCE_H
#include <QtNetwork/QNetworkRequest>
#include "ManagedWallet.h"

//
TM_CORE_BEGIN(am)

//
class ManagedWallet_Binance : public ManagedWallet
{
public:
    ManagedWallet_Binance(AssetsManager* assetsManager,QJsonObject config);

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
#endif // TMIOCORE_MANAGEDWALLET_BINANCE_H
