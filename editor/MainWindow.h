#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <rest/Request.h>
#include "forms/widgets/AssetsHoldingsWidget.h"
#include "forms/views/MdiArea.h"
#include "libs/md/cmc/CoinMarketCap.h"
#include <QMainWindow>
#include <QComboBox>
#include <QtNetwork/QNetworkReply>
#include <QTreeView>
#include <QToolBox>
#include <QToolButton>
#include <QLabel>

//
class QStandardItemModel;


namespace Ui {
class MainWindow;
}

//
class CommunityWidget;
class NewsFeedWidget;
class ExchangeFeedsWidget;

//
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    struct                  WatchAsset
    {
        QString             assetID;
        QAction*            action;
        bool                iconFound;
    };

public:
    void                            createMdiView(QAction*);
    void                            openAssetProperties(const QString& assetID);
    //void                            openAssetCommunity(const QString& assetID);
    QString                         currentAssetProperties();
    void                            openBrowserURL(const QString& url,const QString& title);

private:
    void                            buildToolbar();
    void                            buildToolbarWatches();

protected:
    void                            closeEvent(QCloseEvent *event);

public slots:
    void                            onCurrentAccountResult(tmio::rest::RequestResult&);
    void                            onActionMenuAbout();
    void                            onActionMenuExit();
    void                            onActionMenuAdminAccounts();
    void                            onActionAssetsSummary();
    void                            onActionDashboard();
    void                            onActionBitcoinBlockExplorer();
    void                            onActionExchangeBinanceLiveView();
    void                            onActionWalletsConfigure();
    void                            onLog(tmio::Core::eLogType,QString);
    void                            onMdiViewActivated(QString);
    void                            onToolboxAction(QAction*);
    void                            onRefreshToolbar();
    void                            onOpenMarketDataAssets();
    void                            onOpenWatchAsset();
    void                            onOpenUserProfile();
    void                            onAccountChanged();
    void                            onWatchAssetChanged(const QString& assetID,bool favorite);

private:
    Ui::MainWindow*                 ui;
    QAction*                        _actionAccount              =nullptr;
    QAction*                        _actionWatchDummy           =nullptr;
    ExchangeFeedsWidget*            _exchangeFeedsWidget        =nullptr;
    QComboBox*                      _quoteAssetCombo            =nullptr;
    QTreeView*                      _logsWidget                 =nullptr;
    QStandardItemModel*             _logsModel                  =nullptr;
    MdiArea*                        _mdiArea                    =nullptr;
    QDockWidget*                    _dockProperties             =nullptr;
    QLabel*                         _clockLabel                 =nullptr;
    CommunityWidget*                _communityWidget            =nullptr;
    AssetsHoldingsWidget*           _holdingsWidget             =nullptr;
    NewsFeedWidget*                 _newsFeedWidget             =nullptr;
    QVector<WatchAsset>             _watchAssets;
    QToolButton*                    _marketDataAssets           =nullptr;
    QString                         _currentAssetProperties;
};

#endif // MAINWINDOW_H
