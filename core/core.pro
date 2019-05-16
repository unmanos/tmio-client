#-------------------------------------------------
#
# Project created by QtCreator 2017-05-28T11:34:18
#
#-------------------------------------------------

QT += gui

TARGET = core
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += Core.cpp \
    rest/Connection.cpp \
    rest/Request.cpp \
    api/Account.cpp \
    rest/RequestResult.cpp \
    api/Contact.cpp \
    md/ExchangeFeed.cpp \
    md/MarketData.cpp \
    am/Wallet.cpp \
    am/ManagedWallet.cpp \
    am/AssetsManager.cpp \
    am/ManagedWallet_Binance.cpp \
    am/ManagedWallet_Bitstamp.cpp \
    am/ManagedWallet_Kraken.cpp \
    fc/Math.cpp \
    am/ManagedWallet_Coinbase.cpp \
    md/Asset.cpp \
    md/Source.cpp \
    md/Exchange.cpp \
    cm/Channel.cpp \
    cm/Subscriber.cpp \
    am/PrivateWallet.cpp \
    am/PrivateWallet_Blockchain.cpp \
    am/PrivateWallet_Etherscan.cpp \
    am/PrivateWallet_Blockcypher.cpp \
    am/MiningWallet_Nicehash.cpp \
    am/MiningWallet.cpp \
    am/MiningWallet_Ethermine.cpp \
    nws/Feed.cpp

HEADERS += Core.h \
    rest/rest.h \
    rest/Connection.h \
    rest/Request.h \
    api/Account.h \
    rest/RequestResult.h \
    api/Contact.h \
    md/ExchangeFeed.h \
    md/md.h \
    md/MarketData.h \
    am/Wallet.h \
    am/am.h \
    am/ManagedWallet.h \
    am/AssetsManager.h \
    am/ManagedWallet_Binance.h \
    am/ManagedWallet_Bitstamp.h \
    am/ManagedWallet_Kraken.h \
    fc/Math.h \
    fc/fc.h \
    am/ManagedWallet_Coinbase.h \
    md/Asset.h \
    md/Source.h \
    md/Exchange.h \
    cm/Channel.h \
    cm/cm.h \
    cm/Subscriber.h \
    am/PrivateWallet.h \
    am/PrivateWallet_Blockchain.h \
    am/PrivateWallet_Etherscan.h \
    am/PrivateWallet_Blockcypher.h \
    am/MiningWallet_Nicehash.h \
    am/MiningWallet.h \
    am/MiningWallet_Ethermine.h \
    nws/nws.h \
    nws/Feed.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
