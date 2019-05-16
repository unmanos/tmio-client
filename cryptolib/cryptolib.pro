#-------------------------------------------------
#
# Project created by QtCreator 2016-11-05T10:06:30
#
#-------------------------------------------------

QT       -= gui

TARGET = cryptolib
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        cryptolib.cpp \
    bcrypt/wrapper.c \
    bcrypt/crypt_gensalt.c \
    bcrypt/crypt_blowfish.c \
    bcrypt/bcrypt.c \
    aes/QAESEncryption.cpp

HEADERS += \
        cryptolib.h \
    bcrypt/ow-crypt.h \
    bcrypt/crypt_gensalt.h \
    bcrypt/crypt_blowfish.h \
    bcrypt/crypt.h \
    bcrypt/BCrypt.hpp \
    bcrypt/bcrypt.h \
    aes/QAESEncryption.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
