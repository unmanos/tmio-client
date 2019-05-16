#ifndef TMIOCORE_ASSET_H
#define TMIOCORE_ASSET_H
#include <QObject>
#include <QJsonObject>
#include <QIcon>
#include <rest/RequestResult.h>
#include "md.h"

TM_CORE_BEGIN(md)

//
class MarketData;

//
class Asset : public QObject
{
    Q_OBJECT
public:
    explicit Asset(const QString& id,MarketData* marketData);

    struct                  Quote
    {
        QDateTime       time;
        QString         exchange;
        QString         quoteID;
        double          price;
        bool            reverse;
    };

    static const QString            BTC;
    static const QString            LTC;
    static const QString            ETH;
    static const QString            XMR;
    static const QString            EUR;
    static const QString            USD;
    static const QString            typeFiat;
    static const QString            typeCurrency;
    static const QString            typeToken;

    const QString&                  id();
    bool                            ready();
    const QString&                  name();
    void                            setName(const QString&);
    const QString&                  sign();
    const QString&                  symbol();
    void                            setSymbol(const QString&);
    const QString&                  type();
    bool                            isFiat();
    void                            setType(const QString&);
    const QMap<QString,QString>&    sources();
    const QString&                  source(const QString& name);
    void                            setSource(const QString& name,const QString& coinId);

    const QMap<QString,QString>&    exchanges();
    const QString&                  exchange(const QString& name);
    void                            setExchange(const QString& name,const QString& symbol);

    QIcon                           icon();

    void                            updatePrice(const QString& exchange,const QDateTime& time,const QString& quoteID,double price,bool reverse);
    int                             getPrice(const QString& quoteID,double& price);
    QDateTime                       lastPriceUpdate();

    const QMap<QString,Quote>&      quotes();

    QJsonObject                     data();

private:
    void                            load(QJsonObject);
    QJsonObject                     save();

signals:
    void                            priceChanged();

public slots:
    void                            onReceiveIcon(rest::RequestResult&);

private:
    MarketData*                     _marketData     =nullptr;
    QString                         _id;
    QString                         _name;
    QString                         _sign;
    QString                         _symbol;
    QString                         _type;
    QMap<QString,QString>           _sources;
    QMap<QString,QString>           _exchanges;
    QIcon                           _icon;
    bool                            _iconLoaded     =false;
    rest::Request*                  _iconRequest    =nullptr;
    QMap<QString,Quote>             _quotes;
    QJsonObject                     _data;

    friend class                    MarketData;
};

TM_CORE_END()
#endif // TMIOCORE_ASSET_H
