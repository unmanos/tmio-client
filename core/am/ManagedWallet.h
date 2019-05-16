#ifndef TMIOCORE_MANAGEDWALLET_H
#define TMIOCORE_MANAGEDWALLET_H
#include "Wallet.h"

TM_CORE_BEGIN(am)

//
class ManagedWallet : public Wallet
{
public:
    ManagedWallet(AssetsManager* assetsManager,QJsonObject config);

protected:
    int             _updateInterval         =(60*2);
};

TM_CORE_END()

#endif // TMIOCORE_MANAGEDWALLET_H
