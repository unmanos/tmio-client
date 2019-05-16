#ifndef TMIOCORE_MININGWALLET_NICEHASH_H
#define TMIOCORE_MININGWALLET_NICEHASH_H
#include <QtNetwork/QNetworkRequest>
#include "MiningWallet.h"
TM_CORE_BEGIN(am)

//
class MiningWallet_Nicehash : public MiningWallet
{
public:
    MiningWallet_Nicehash(AssetsManager* assetsManager,QJsonObject config);

private:
    void                    updateBalance();

public slots:
    void                    onUpdate();
    void                    onReceiveBalance();

private:
    QString                 _apiID;
    QString                 _apiKey;
    QNetworkRequest*        _balanceRequest;
    double                  _confirmedBalance;
    double                  _pendingBalance;
};

TM_CORE_END()
#endif // TMIOCORE_MININGWALLET_NICEHASH_H
