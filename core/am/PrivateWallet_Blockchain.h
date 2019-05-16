#ifndef PRIVATEWALLET_BLOCKCHAIN_H
#define PRIVATEWALLET_BLOCKCHAIN_H
#include "PrivateWallet.h"
TM_CORE_BEGIN(am)

//
class PrivateWallet_Blockchain : public PrivateWallet
{
public:
    explicit PrivateWallet_Blockchain(AssetsManager* assetsManager,QJsonObject config);

public slots:
    void                        onUpdate();
    void                        onReceive();

private:
    QMap<QString,QJsonObject>   _addresses;
};

TM_CORE_END()
#endif // PRIVATEWALLET_BLOCKCHAIN_H
