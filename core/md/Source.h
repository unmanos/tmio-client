#ifndef TMIOCORE_SOURCE_H
#define TMIOCORE_SOURCE_H
#include <QObject>
#include <QIcon>
#include "md.h"
#include "../rest/Connection.h"
#include "../rest/Request.h"

TM_CORE_BEGIN(md)

//
class Source : public QObject
{
    Q_OBJECT
public:
    explicit Source(rest::Connection* connection,const QString& name,QObject *parent = nullptr);

    struct                  Coin
    {
        QString             id;
        QString             symbol;
        QString             name;
        QIcon               icon;
        qint64              circulatingSupply;
        double              priceBTC;
    };

    const QString&              name();
    bool                        ready();
    const QMap<QString,Coin>&   coins();
    const Coin&                 coin(const QString& id);
    QString                     coinIdFromName(const QString& name);

signals:

public slots:
    void                        onReceiveCoins(tmio::rest::RequestResult&);
    void                        onReceiveCoinIcon(tmio::rest::RequestResult&);

private:
    QString                     _name;
    tmio::rest::Request*        _request        =nullptr;
    QMap<QString,Coin>          _coins;
    rest::Connection*           _connection;
    int                         _loadingIcons   =0;
};

TM_CORE_END()
#endif // TMIOCORE_SOURCE_H
