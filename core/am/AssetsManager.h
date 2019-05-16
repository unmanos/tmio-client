#ifndef TMIOCORE_ASSETSMANAGER_H
#define TMIOCORE_ASSETSMANAGER_H
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include "Core.h"
#include "md/MarketData.h"
#include "Wallet.h"

TM_CORE_BEGIN(am)

//
class AssetsManager : public QObject
{
    Q_OBJECT
public:
    explicit AssetsManager(tmio::md::MarketData*);

    void                                reset();
    tmio::md::MarketData*               marketData();
    QVector<Wallet*>                    wallets();
    void                                addWallet(QJsonObject config);
    QList<Wallet::Asset>                assets();

    QNetworkAccessManager*              networkManager();
    QNetworkRequest*                    createNetworkRequest(const QUrl& url);


signals:
    void                                walletsChanged();

public slots:
    void                                onWalletAssetsChanged();

private:
    tmio::md::MarketData*               _marketData;
    QNetworkAccessManager*              _networkManager;
    QVector<Wallet*>                    _wallets;
};

TM_CORE_END()
#endif // TMIOCORE_ASSETSMANAGER_H
