#ifndef TMIOCORE_MANAGEDWALLET_KRAKEN_H
#define TMIOCORE_MANAGEDWALLET_KRAKEN_H
#include <QObject>
#include "ManagedWallet.h"

//
class QNetworkRequest;

//
TM_CORE_BEGIN(am)

//
class ManagedWallet_Kraken : public ManagedWallet
{
public:
    ManagedWallet_Kraken(AssetsManager* assetsManager,QJsonObject config);

private:
    QByteArray              getHMAC(const QString & message);
    void                    addHeaders(QNetworkRequest* request,qint64 nonce, QString postData, QString path);
    void                    getAccount();

public slots:
    void                    onUpdate();
    void                    onReceiveBalance();

private:
    QString                 _apiKey;
    QString                 _apiSecret;
};

TM_CORE_END()
#endif // TMIOCORE_MANAGEDWALLET_KRAKEN_H
