#ifndef TMIOCORE_MATH_H
#define TMIOCORE_MATH_H
#include "fc.h"
TM_CORE_BEGIN(fc)

class Math
{
public:
    Math();

    static double               SMA(const QVector<double>& arr,int interval);
    static double               EMA(const QVector<double>& prices,int interval,QVector<double>& previous);
    static double               VWMA(const QVector<double>& prices,const QVector<double>& volumes,int interval);
    static double               DEV(const QVector<double>& arr,int interval);
};

TM_CORE_END()

#endif // TMIOCORE_MATH_H
