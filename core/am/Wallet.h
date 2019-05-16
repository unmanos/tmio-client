#ifndef TMIOCORE_WALLET_H
#define TMIOCORE_WALLET_H
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include "am.h"
#include "md/MarketData.h"

TM_CORE_BEGIN(am)

//
class AssetsManager;

//
class Wallet : public QObject
{
    Q_OBJECT
public:
    explicit Wallet(AssetsManager*,QJsonObject);

    struct Asset
    {
        QString         symbol;
        QString         assetID;
        double          totalAmount;
    };

    QJsonObject                 config();
    int                         id();
    QString                     name();

protected:
    AssetsManager*              manager();
    tmio::md::MarketData*       marketData();
    QNetworkAccessManager*      networkManager();
    void                        resetAssets();
    void                        setAsset(QString symbol,QString assetID,double amount);

public:
    QVector<Asset>              assets();

signals:
    void                        assetsChanged();

public slots:

private:
    AssetsManager*              _assetsManager  =nullptr;
    QJsonObject                 _config;
    int                         _id             =0;
    QString                     _name;
    QMap<QString,Asset>         _assets;
};

TM_CORE_END()

#endif // TMIOCORE_WALLET_H
