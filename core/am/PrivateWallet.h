#ifndef PRIVATEWALLET_H
#define PRIVATEWALLET_H
#include "Wallet.h"

TM_CORE_BEGIN(am)

//
class PrivateWallet : public Wallet
{
    Q_OBJECT
public:
    explicit PrivateWallet(AssetsManager* assetsManager,QJsonObject config);

protected:
    int             _updateInterval         =(60*2);

signals:

public slots:
};

TM_CORE_END()

#endif // PRIVATEWALLET_H
