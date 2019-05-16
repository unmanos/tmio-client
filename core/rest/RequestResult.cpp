#include "RequestResult.h"
TM_CORE_BEGIN(rest)

//
RequestResult::RequestResult(Request* req)
{
    _statusCode=0;
    _request=req;
}

//
Request* RequestResult::request()
{
    return(_request);
}

TM_CORE_END()
