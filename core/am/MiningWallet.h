#ifndef MININGWALLET_H
#define MININGWALLET_H
#include "Wallet.h"
TM_CORE_BEGIN(am)

//
class MiningWallet : public Wallet
{
    Q_OBJECT
public:
    explicit MiningWallet(AssetsManager* assetsManager,QJsonObject config);

protected:
    int             _updateInterval         =(60*2);

signals:

public slots:
};

TM_CORE_END()
#endif // MININGWALLET_H
