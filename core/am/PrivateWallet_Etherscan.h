#ifndef PRIVATEWALLET_ETHERSCAN_H
#define PRIVATEWALLET_ETHERSCAN_H
#include "PrivateWallet.h"
TM_CORE_BEGIN(am)

class PrivateWallet_Etherscan : public PrivateWallet
{
public:
    PrivateWallet_Etherscan(AssetsManager* assetsManager,QJsonObject config);

public slots:
    void                        onUpdate();
    void                        onReceive();

private:
    QMap<QString,QJsonObject>   _addresses;
};

TM_CORE_END()
#endif // PRIVATEWALLET_ETHERSCAN_H
