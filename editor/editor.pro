#-------------------------------------------------
#
# Project created by QtCreator 2016-08-14T23:54:17
#
#-------------------------------------------------

QT       += core gui network
QT       += websockets
#QT       += xml
QT       += widgets
QT       += multimedia
QT       -= qml

# https://bugreports.qt.io/browse/QTBUG-52501
#CONFIG += static
#export LD_LIBRARY_PATH=/usr/local/Qt/5.5/gcc_64/lib:$LD_LIBRARY_PATH
#DEPENDPATH += /usr/local/Qt/5.5/gcc_64/lib
#DEPENDPATH += usr/lib/x86_64-linux-gnu
DEPENDPATH += /usr/lib/x86_64-linux-gnu
#CONFIG += staticlib
#QMAKE_CXXFLAGS += -static
#QMAKE_LFLAGS += -static

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter


#greaterThan(QT_MAJOR_VERSION, 5): QT += widgets quickwidgets location svg

TARGET = editor
TEMPLATE = app

PRECOMPILED_HEADER = pch/stable.h

#QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

#TRANSLATIONS = editor_fr.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        MainWindow.cpp \
    forms/system/LoginDialog.cpp \
    Application.cpp \
    forms/views/MdiArea.cpp \
    forms/views/tickers/TickerPriceView.cpp \
    libs/ui/charts/FinChartWidget.cpp \
    libs/ui/exchange/MarketWatchWidget.cpp \
    libs/ui/exchange/MarketWatchListWidget.cpp \
    libs/ui/sort/GenericSortFilterProxyModel.cpp \
    libs/md/cmc/CoinMarketCap.cpp \
    forms/system/LoadingDialog.cpp \
    forms/widgets/CoinInfoWidget.cpp \
    forms/views/coins/CoinMarketCapView.cpp \
    forms/views/tickers/ChartSettingsPopup.cpp \
    libs/ui/charts/FinChartData.cpp \
    libs/ui/charts/FinChartDataCandles.cpp \
    forms/views/mining/NicehashView.cpp \
    Settings.cpp \
    forms/views/assets/AssetsSummaryView.cpp \
    forms/system/UserProfileDialog.cpp \
    forms/views/markets/ExchangeLiveView.cpp \
    Style.cpp \
    forms/system/AboutDialog.cpp \
    forms/views/coins/CoinDetailView.cpp \
    forms/views/assets/MarketDataAssetsView.cpp \
    forms/admin/MarketDataEditAssetDialog.cpp \
    forms/widgets/CommunityWidget.cpp \
    forms/wallets/ManageWalletsDialog.cpp \
    forms/wallets/ConfigureWalletsDialog.cpp \
    forms/wallets/ConfigureWalletWidget.cpp \
    forms/widgets/CommunityPageAsset.cpp \
    forms/views/assets/AssetView.cpp \
    libs/ui/asset/AssetPairsWidget.cpp \
    libs/ui/community/CommunityChannel.cpp \
    forms/widgets/AssetWidget.cpp \
    forms/widgets/AssetsHoldingsWidget.cpp \
    libs/ui/style/IconnedDockStyle.cpp \
    forms/widgets/NewsFeedWidget.cpp \
    libs/ui/charts/PieChart.cpp \
    forms/views/browser/BrowserView.cpp \
    forms/widgets/ExchangeFeedsWidget.cpp \
    forms/views/admin/AccountsManagerView.cpp \
    forms/views/dashboard/DashboardView.cpp \
    libs/ui/charts/LineChart.cpp \
    libs/ui/charts/ChartUtils.cpp \
    forms/views/explorers/BitcoinBlockExplorerView.cpp \
    forms/widgets/BitcoinBlocksWidget.cpp

HEADERS  += MainWindow.h \
    forms/system/LoginDialog.h \
    Application.h \
    pch/stable.h \
    forms/views/MdiArea.h \
    forms/views/tickers/TickerPriceView.h \
    libs/ui/charts/FinChartWidget.h \
    libs/ui/exchange/MarketWatchWidget.h \
    libs/ui/exchange/MarketWatchListWidget.h \
    libs/ui/sort/GenericSortFilterProxyModel.h \
    libs/md/cmc/CoinMarketCap.h \
    forms/system/LoadingDialog.h \
    forms/widgets/CoinInfoWidget.h \
    forms/views/coins/CoinMarketCapView.h \
    forms/views/tickers/ChartSettingsPopup.h \
    libs/ui/charts/FinChartData.h \
    libs/ui/charts/FinChartDataCandles.h \
    forms/views/mining/NicehashView.h \
    Settings.h \
    forms/views/assets/AssetsSummaryView.h \
    forms/system/UserProfileDialog.h \
    forms/views/markets/ExchangeLiveView.h \
    Style.h \
    forms/system/AboutDialog.h \
    forms/views/coins/CoinDetailView.h \
    forms/views/assets/MarketDataAssetsView.h \
    forms/admin/MarketDataEditAssetDialog.h \
    forms/widgets/CommunityWidget.h \
    forms/wallets/ManageWalletsDialog.h \
    forms/wallets/ConfigureWalletsDialog.h \
    forms/wallets/ConfigureWalletWidget.h \
    config.h \
    forms/widgets/CommunityPageAsset.h \
    forms/views/assets/AssetView.h \
    libs/ui/asset/AssetPairsWidget.h \
    libs/ui/community/CommunityChannel.h \
    forms/widgets/AssetWidget.h \
    forms/widgets/AssetsHoldingsWidget.h \
    libs/ui/style/IconnedDockStyle.h \
    forms/widgets/NewsFeedWidget.h \
    libs/ui/charts/PieChart.h \
    forms/views/browser/BrowserView.h \
    forms/widgets/ExchangeFeedsWidget.h \
    forms/views/admin/AccountsManagerView.h \
    forms/views/dashboard/DashboardView.h \
    libs/ui/charts/LineChart.h \
    libs/ui/charts/ChartUtils.h \
    forms/views/explorers/BitcoinBlockExplorerView.h \
    forms/widgets/BitcoinBlocksWidget.h

FORMS    += MainWindow.ui \
    forms/system/LoginDialog.ui \
    forms/views/MdiArea.ui \
    forms/widgets/ToolBox.ui \
    forms/views/tickers/TickerPriceView.ui \
    forms/system/LoadingDialog.ui \
    forms/views/tickers/ChartSettingsPopup.ui \
    forms/system/UserProfileDialog.ui \
    forms/system/AboutDialog.ui \
    forms/admin/MarketDataEditAssetDialog.ui \
    forms/wallets/ManageWalletsDialog.ui \
    forms/wallets/ConfigureWalletsDialog.ui

RESOURCES += \
    res/icons.qrc \
    localization.qrc \
    res/web.qrc \
    res/map.qrc \
    res/sounds.qrc

DISTFILES += \
    editor_fr.qm \
    res/web/gmap.html


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libcore.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cryptolib/release/ -lcryptolib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../cryptolib/debug/ -lcryptolib
else:unix: LIBS += -L$$OUT_PWD/../cryptolib/ -lcryptolib

INCLUDEPATH += $$PWD/../cryptolib
DEPENDPATH += $$PWD/../cryptolib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptolib/release/libcryptolib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptolib/debug/libcryptolib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptolib/release/cryptolib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptolib/debug/cryptolib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../cryptolib/libcryptolib.a
