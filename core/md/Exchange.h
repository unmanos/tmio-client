#ifndef TMIOCORE_EXCHANGE_H
#define TMIOCORE_EXCHANGE_H
#include <QObject>
#include "md.h"

TM_CORE_BEGIN(md)

//
class ExchangeFeed;

//
class MarketData;

//
class Exchange : public QObject
{
    Q_OBJECT
public:
    explicit Exchange(const QString& id,const QString& name,bool feed,MarketData* marketData);

    const QString&          id();
    const QString&          name();
    bool                    ready();
    ExchangeFeed*           feed();
    QStringList             quoteAssets();
    QStringList             quoteSymbols();
    QStringList             symbols();
    QStringList             unregisteredSymbols();

signals:

public slots:

private:
    MarketData*             _marketData =nullptr;
    QString                 _id;
    QString                 _name;
    ExchangeFeed*           _feed       =nullptr;
    QMap<QString,bool>      _favoritesTickers;
};

TM_CORE_END()
#endif // TMIOCORE_EXCHANGE_H
