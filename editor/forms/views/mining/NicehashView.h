#ifndef NICEHASHVIEW_H
#define NICEHASHVIEW_H
#include <QLabel>
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>

#include "libs/ui/sort/GenericSortFilterProxyModel.h"
#include <rest/RequestResult.h>
#include <rest/Request.h>

//
class NicehashView : public QWidget
{
    Q_OBJECT
public:
    explicit NicehashView(QString apiAddress,QWidget *parent = nullptr);

    struct                      AlgoStat
    {
        double          balance;
        int             algo;
        double          acceptedSpeed;
        double          rejectedSpeed;
    };

private:
    QString                     formatHashRate(quint64);
    void                        updateUI();

signals:

public slots:
    void                        onUpdate();
    void                        onReceiveStats();
    void                        onQuoteAssetChanged();

private:
    QString                     _apiAddress;
    QLabel*                     _labelPendingBalance;
    QLabel*                     _labelPendingBalanceFiat;
    QLabel*                     _labelLastPaymentsAmount;
    QLabel*                     _labelLastPaymentsAmountFiat;
    QNetworkRequest*            _statsRequest;
    double                      _pendingBalance;
    double                      _lastPaymentsAmount;
    QTreeView*                  _treeView;
    GenericSortFilterProxyModel*_sortFilter;
    QStandardItemModel*         _algosModel;
    QMap<int,QString>           _algos;
    QVector<AlgoStat>           _algoStats;
};

#endif // NICEHASHVIEW_H
