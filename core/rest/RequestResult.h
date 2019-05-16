#pragma once
#include "rest.h"
#include <QJsonDocument>
TM_CORE_BEGIN(rest)

class Request;

class RequestResult
{
public:
    RequestResult(Request*);
    Request*    request();

public:
    Request* _request;
    int _statusCode;
    QJsonDocument _document;
    QByteArray _raw;
};


TM_CORE_END()
