#ifndef PRIVATEWALLET_BLOCKCYPHER_H
#define PRIVATEWALLET_BLOCKCYPHER_H
#include "PrivateWallet.h"
TM_CORE_BEGIN(am)

//
class PrivateWallet_Blockcypher : public PrivateWallet
{
public:
    PrivateWallet_Blockcypher(AssetsManager* assetsManager,QJsonObject config);
};

TM_CORE_END()
#endif // PRIVATEWALLET_BLOCKCYPHER_H
