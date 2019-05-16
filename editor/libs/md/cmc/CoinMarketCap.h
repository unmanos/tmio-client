#ifndef COINMARKETCAP_H
#define COINMARKETCAP_H
#include <QObject>
#include <QMap>
#include <QIcon>
#include <rest/Connection.h>

//
class CoinMarketCap : public QObject
{
    Q_OBJECT
public:
    explicit CoinMarketCap(QObject*parent = nullptr);

    bool                    ready();
    QString                 quoteAssetID();
    QString                 quoteAssetSymbol();
    void                    setQuoteAssetID(const QString& fiatID);
    QString                 formatFiatCurrency(double amount,QString fiatID);
    QString                 formatAmount(double amount,double precision);
    QString                 formatSupply(double supply);
    //double                  assetPrice(const QString& quoteAssetID,const QString& assetID,double amount=1);
    bool                    assetPrice(const QString& quoteAssetID,const QString& assetID,double& amount,QVector<QString>* route,int recurse=3);
    double                  circulatingSupply(const QString& assetID);
    double                  totalMarketCap(const QString& quoteAssetID);
    QIcon                   assetIcon(const QString assetID);
    int                     assetRank(const QString assetID);
    QIcon                   exchangeIcon(const QString exchangeID);
    QIcon                   miningPoolIcon(const QString miningPoolID);
    QString                 miningPoolName(const QString miningPoolID);
signals:
    void                    quoteAssetChanged();

public slots:

private:
    QString                 _quoteAssetID;
    QMap<QString,int>       _assetRanks;
};

#endif // COINMARKETCAP_H
