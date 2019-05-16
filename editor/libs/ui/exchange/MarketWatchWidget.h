#ifndef MARKETWATCHWIDGET_H
#define MARKETWATCHWIDGET_H
#include <QWidget>
#include <QTreeWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QComboBox>
#include <QTabWidget>
#include <QCheckBox>
#include <rest/Connection.h>
#include "libs/md/cmc/CoinMarketCap.h"
#include <md/MarketData.h>
#include <md/ExchangeFeed.h>

//
class QSortFilterProxyModel;

//
class MarketWatchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MarketWatchWidget(const QString& exchangeID,QWidget *parent = 0);

    struct      Currency
    {
        QString                 symbol;
        //int                     fiatID;
        QString                 assetID;
        QString                 assetName;
        QTreeView*              pairs;
        QStandardItemModel*     model;
        QSortFilterProxyModel*  sortFilter;
        int                     volumeDivider;
        int                     precision;
        Currency() {pairs=NULL;model=NULL;volumeDivider=1;precision=8;}
        Currency(const QString& symbol,QString assetID,QString assetName,int precision) {this->symbol=symbol;this->assetID=assetID;this->assetName=assetName;pairs=NULL;model=NULL;volumeDivider=1;this->precision=precision;};
    };

signals:
    void                    openTicker(QString exchange,QString ticker,QString assetID);

public slots:
    void                    onRefreshTickers();
    void                    onCurrentRowChanged(QModelIndex,QModelIndex);
    void                    onOpenTicker(QModelIndex);
    void                    onChangePeriodUpdate(int);
    void                    onChangeFilter();
    void                    onTickerChanged(const tmio::md::ExchangeFeed::Ticker& ticker);
    void                    onItemChanged(QStandardItem*);
    void                    onFavoriteTickerChanged(const QString& exchange,const QString& ticker,bool favorite);
    void                    onCurrentQuoteChanged(int);

private:
    void                    updateData(bool resetChange);
    void                    updateTicker(const tmio::md::ExchangeFeed::Ticker& ticker);

private:
    QString                 _exchange;
    QList<Currency>         _currencies;
    QComboBox*              _columnType;
    QComboBox*              _changePeriod;
    QComboBox*              _volumePeriod;
    QCheckBox*              _filterTop100;
    QTabWidget*             _currenciesTab;
    QJsonArray              _tickers;
    tmio::md::MarketData*   _marketData;
    tmio::md::ExchangeFeed* _feed;
    bool                    _externalUpdate     =false;
};

#endif // MARKETWATCHWIDGET_H
