#include "Source.h"
TM_CORE_BEGIN(md)

#define TAG                            QString("md::Source(%1)").arg(_name)
#define API_COIN_LIST_URL              "marketdata/sources/%1/assets"
#define API_COIN_ICON_URL              "marketdata/sources/%1/assets/%2/icon"

#define REQEUEST_PROPERTY_COINID       "reqpropcoinid"

//
Source::Source(rest::Connection* connection,const QString& name,QObject *parent) : QObject(parent)
{
    _name           =name;
    _connection     =connection;
    _request        =connection->createRequest(QString(API_COIN_LIST_URL).arg(name));
    connect(_request,&rest::Request::finished,this,&Source::onReceiveCoins);
    _request->get(false);
}

//
const QString&                      Source::name()
{
    return(_name);
}

//
bool                                Source::ready()
{
    return(_request==nullptr);
}

//
const QMap<QString,Source::Coin>&   Source::coins()
{
    return(_coins);
}

//
const Source::Coin&                 Source::coin(const QString& id)
{
    if(_coins.contains(id))
    {
        return(_coins[id]);
    }
    static Coin emptyCoin;
    return(emptyCoin);
}

//
QString                             Source::coinIdFromName(const QString& name)
{
    int     match=0;
    QString id;
    foreach(const Coin& c,_coins.values())
    {
        if(c.name==name)
        {
            id=c.id;
            match++;
        }
    }
    if(match!=1)
    {
        TM_ERROR(TAG,"Coin name'"+name+"' have match!=1");
    }
    return((match==1)?id:"");
}

//
void                                Source::onReceiveCoins(tmio::rest::RequestResult& result)
{
    if(result._statusCode==200)
    {
        QJsonArray coins=result._document.array();
        TM_TRACE(TAG,QString("Received %1 coins").arg(coins.count()));
        for(int i=0;i<coins.count();i++)
        {
            QJsonObject     coinjs=coins.at(i).toObject();
            QJsonObject     quotes=coinjs["quotes"].toObject();
            Coin            coin;
            coin.id                 =coinjs["_id"].toString();
            coin.name               =coinjs["name"].toString();
            coin.symbol             =coinjs["symbol"].toString();
            coin.circulatingSupply  =0;
            if(coinjs.contains("circulatingSupply"))
            {
                coin.circulatingSupply  =coinjs["circulatingSupply"].toVariant().toLongLong();
            }
            coin.priceBTC           =0;
            if(coinjs.contains("quotes"))
            {
                if(coinjs["quotes"].toObject().contains("BTC"))
                {
                    coin.priceBTC   =coinjs["quotes"].toObject()["BTC"].toObject()["price"].toDouble();
                }
            }


            _coins.insert(coin.id,coin);
            // reuqest icon
            rest::Request*  iconRequest=_connection->createRequest(QString(API_COIN_ICON_URL).arg(_name).arg(coin.id));
            iconRequest->setProperty(REQEUEST_PROPERTY_COINID,coin.id);
            connect(iconRequest,&rest::Request::finished,this,&Source::onReceiveCoinIcon);
            iconRequest->get(false);
            _loadingIcons++;
        }
    }
    else
    {
        TM_TRACE(TAG,QString("Error receiving coins (status %1)").arg(result._statusCode));
    }
    delete(_request);
    _request=nullptr;
}

//
void                                Source::onReceiveCoinIcon(tmio::rest::RequestResult& result)
{
    if(result._statusCode==200)
    {
        QString     id  =result.request()->property(REQEUEST_PROPERTY_COINID).toString();
        Coin&       coin=_coins[id];
        QImage      image;
        //image.load("/home/pierre/mur en pierre.JPG");
        image.loadFromData(result._raw);
        QPixmap buffer = QPixmap::fromImage(image);
        //QIcon icon;
        coin.icon.addPixmap(buffer);
    }
    // set icon
    delete(result.request());
    _loadingIcons--;
}

TM_CORE_END()
