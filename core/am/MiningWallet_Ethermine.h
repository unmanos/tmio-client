#ifndef MININGWALLET_ETHERMINE_H
#define MININGWALLET_ETHERMINE_H
#include "MiningWallet.h"
TM_CORE_BEGIN(am)

//
class MiningWallet_Ethermine : public MiningWallet
{
public:
    MiningWallet_Ethermine(AssetsManager* assetsManager,QJsonObject config);

private:
    void                    updateBalance();

public slots:
    void                    onUpdate();
    void                    onReceiveBalance();

private:
    QString                 _address;
    QNetworkRequest*        _balanceRequest     =nullptr;
};

TM_CORE_END()
#endif // MININGWALLET_ETHERMINE_H
