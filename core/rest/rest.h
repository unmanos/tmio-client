#pragma once
#ifndef TMIOCORE_REST_H
#define TMIOCORE_REST_H
#include "../Core.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

TM_CORE_BEGIN(rest)

const quint32           version=0x0100;
const int               defaultRequestVersion=1;

TM_CORE_END()
#endif // TMIOCORE_REST_H
