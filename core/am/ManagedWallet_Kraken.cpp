#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include "ManagedWallet_Kraken.h"

//
#define    TAG              "tmio::am::ManagedWallet_Kraken"

//
#define    API_URL          "https://api.kraken.com"
#define    TRADE_BALANCE    "/0/private/Balance"

//
TM_CORE_BEGIN(am)

//
ManagedWallet_Kraken::ManagedWallet_Kraken(AssetsManager* assetsManager,QJsonObject config) : ManagedWallet(assetsManager,config)
{
    _apiKey     =config["exchange"].toObject()["kraken"].toObject()["apiKey"].toString();
    _apiSecret  =config["exchange"].toObject()["kraken"].toObject()["apiSecret"].toString();
    // setup timer
    QTimer*             updateTimer=new QTimer();
    connect(updateTimer,&QTimer::timeout,this,&ManagedWallet_Kraken::onUpdate);
    updateTimer->start(1000*_updateInterval);
    onUpdate();
}

//
QByteArray                      ManagedWallet_Kraken::getHMAC(const QString & message)
{
    QUrl url(message);
    QByteArray  enc=QMessageAuthenticationCode::hash(url.toEncoded(), QByteArray(), QCryptographicHash::Sha256).toHex();
    QByteArray  secret=QByteArray::fromBase64(_apiSecret.toLocal8Bit());

    return QMessageAuthenticationCode::hash(enc, secret, QCryptographicHash::Sha512).toHex();
}

//
void                            ManagedWallet_Kraken::addHeaders(QNetworkRequest* request,qint64 nonce, QString postData, QString path)
{
    request->setRawHeader("API-Key", _apiKey.toLocal8Bit());
    QByteArray base64DecodedSecred = QByteArray::fromBase64(_apiSecret.toUtf8());
    QString np = QString::number(nonce);
    np.append(postData);
    QByteArray pathBytes = path.toLocal8Bit();
    QByteArray hash256Bytes = QCryptographicHash::hash(np.toLocal8Bit(),QCryptographicHash::Sha256);
    QByteArray z;
    z.append(pathBytes);
    z.append(hash256Bytes);
    QString signature=QMessageAuthenticationCode::hash(z, base64DecodedSecred, QCryptographicHash::Sha512).toBase64(/*QByteArray::Base64Encoding*/);
    request->setRawHeader("API-Sign", signature.toLocal8Bit());
}

//
void                            ManagedWallet_Kraken::getAccount()
{
    QString url(QString(API_URL) + QString(TRADE_BALANCE));
    const qint64  nonce=QDateTime::currentMSecsSinceEpoch()*1000;
    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QUrlQuery postDataUrlQuery;
    postDataUrlQuery.addQueryItem("nonce", QString::number(nonce));

    QByteArray postData=postDataUrlQuery.toString(QUrl::FullyEncoded).toLocal8Bit();
    addHeaders(&netReq,nonce,postData,TRADE_BALANCE);

    QNetworkReply*  reply=networkManager()->post(netReq,postData);

    connect(reply, &QNetworkReply::finished, this, &ManagedWallet_Kraken::onReceiveBalance);
}

//
void                            ManagedWallet_Kraken::onUpdate()
{
    TM_TRACE(TAG,"onUpdate");
    getAccount();
}

//
void                            ManagedWallet_Kraken::onReceiveBalance()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int statusCode=0;
    if(reply->error() == QNetworkReply::NoError)
    {
        TM_TRACE(TAG,"Received account");
        statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode==200)
        {
            QByteArray      raw=reply->readAll();
            QJsonDocument   doc=QJsonDocument::fromJson(raw);
            if(doc.object().contains("result"))
            {
                resetAssets();
                QJsonObject     result=doc.object()["result"].toObject();
                QStringList     keys=result.keys();
                foreach(QString key, keys)
                {
                    const double balance=result[key].toString().toDouble();
                    QString type=key.length()?key.left(1):"";
                    QString symbol=key.length()?(key.right(key.length()-1)):"";
                    if( (symbol.length())&&(balance!=0.) )
                    {
                        if(type=="Z")
                        {
                            // fiat
                            // TODO: remove this match...
                            setAsset(symbol,"fiat-"+symbol.toLower(),balance);
                        }
                        if(type=="X")
                        {
                            // crypto
                            QString assetID=marketData()->loopupExchangeAsset(md::exchangeKraken,symbol);
                            setAsset(symbol,assetID,balance);
                        }
                    }
                }
            }
            TM_TRACE(TAG,raw.toStdString().c_str());
        }
    }
    if(statusCode!=200)
    {
        TM_ERROR(TAG,QString("Failed account (%1)").arg(statusCode));
    }
    delete(reply);
}


TM_CORE_END()
