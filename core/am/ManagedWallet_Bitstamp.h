#ifndef TMIOCORE_MANAGEDWALLET_BITSTAMP_H
#define TMIOCORE_MANAGEDWALLET_BITSTAMP_H
#include <QtNetwork/QNetworkRequest>
#include "ManagedWallet.h"

//
TM_CORE_BEGIN(am)

//
class ManagedWallet_Bitstamp : public ManagedWallet
{
public:
    ManagedWallet_Bitstamp(AssetsManager* assetsManager,QJsonObject config);

private:
    QByteArray              getHMAC(const QString & message);
    void                    getAccount();

public slots:
    void                    onUpdate();
    void                    onReceiveBalance();

private:
    quint64                 _apiNonce;
    QString                 _customerID;
    QString                 _apiKey;
    QString                 _apiSecret;

};

TM_CORE_END()
#endif // TMIOCORE_MANAGEDWALLET_BITSTAMP_H
