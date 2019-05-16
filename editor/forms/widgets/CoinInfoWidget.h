#ifndef COININFOWIDGET_H
#define COININFOWIDGET_H
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QMap>
#include <QModelIndex>
#include "libs/md/cmc/CoinMarketCap.h"

//
class QTabWidget;
class QStandardItemModel;
class QStandardItem;
class QCheckBox;

//
class CoinInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CoinInfoWidget(QString assetID,QWidget *parent = 0);

    void                            buildTabInfo();
    void                            buildTabPairs();
    void                            buildTabChain();
    void                            buildTabSocial();
    void                            refreshTabInfo();
    void                            refreshTabChain();

public slots:
    void                            onRefreshData();
    void                            onCurrentTabChanged(int);
    void                            onChangeWatch(bool);
    void                            onOpenPair(const QString&,const QString&);
    void                            onAssetModified(const QString& id);

signals:
    void                            sigActionTriggered(QAction*);

private:
    QString                         _assetID;
    QTabWidget*                     _tabWidget              =nullptr;
    QFormLayout*                    _infoLayout             =nullptr;
    QFormLayout*                    _chainLayout            =nullptr;
    QLabel*                         _labelMarketCap         =nullptr;
    QLabel*                         _labelPrice             =nullptr;
    QLabel*                         _labelPriceConvert      =nullptr;
    QLabel*                         _labelRank              =nullptr;
    QLabel*                         _labelCirculatingSupply =nullptr;
    QCheckBox*                      _watchCheckbox          =nullptr;
    static int                      _currentPage;
};

#endif // COININFOWIDGET_H
