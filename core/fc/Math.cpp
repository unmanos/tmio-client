#include "Math.h"
#include <math.h>

TM_CORE_BEGIN(fc)

//
Math::Math()
{

}

//
double              Math::SMA(const QVector<double>& arr,int interval)
{
    if(interval>arr.length())
    {
        interval=arr.length()?arr.length():interval;
    }
    if(arr.length()>=interval)
    {
        double v=0;
        for(int i=arr.length()-interval;i<arr.length();i++)
        {
            v+=arr.at(i);
        }
        return(v/((double)interval));
    }
    return(-1);
}

//
double              Math::EMA(const QVector<double>& prices,int interval,QVector<double>& previous)
{
    double ema=0;
    if(prices.length()<interval)
    {
        ema=SMA(prices,interval);
    }
    else
    {
        double weightingMultiplier=2.0/((double)(interval + 1));
        ema=(prices.last()-previous.last()) * weightingMultiplier + previous.last();
    }
    previous.append(ema);
    return(ema);
}

//
double              Math::VWMA(const QVector<double>& prices,const QVector<double>& volumes,int interval)
{
    if(prices.length()==volumes.length())
    {
        if(interval>prices.length())
        {
            interval=prices.length()?prices.length():interval;
        }
        if(prices.length()>=interval)
        {
            double ma=0;
            double volume=0;
            for(int i=prices.length()-interval;i<prices.length();i++)
            {
                ma+=prices.at(i)*volumes.at(i);
                volume+=volumes.at(i);
            }
            return(ma/volume);
        }
    }
    return(-1);
}

//
double              Math::DEV(const QVector<double>& arr,int interval)
{
    // https://www.mathsisfun.com/data/standard-deviation-formulas.html
    if(interval>arr.length())
    {
        interval=arr.length()?arr.length():interval;
    }
    if(arr.length()>=interval)
    {
        double mean=SMA(arr,interval);
        double variance=0;
        for(int i=arr.length()-interval;i<arr.length();i++)
        {
            double      diff=arr.at(i)-mean;
            variance+=(diff*diff);
        }
        if(interval>1)
            interval=interval-1;
        variance/=((double)(interval));
        return(/*deviation*/sqrt(variance));
    }
    return(-1);
}




TM_CORE_END()
