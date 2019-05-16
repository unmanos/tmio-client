#include "ManagedWallet.h"

TM_CORE_BEGIN(am)

ManagedWallet::ManagedWallet(AssetsManager* assetsManager,QJsonObject config) : Wallet(assetsManager,config)
{
}

TM_CORE_END()
