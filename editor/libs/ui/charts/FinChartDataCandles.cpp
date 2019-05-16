#include "FinChartDataCandles.h"
#include "FinChartWidget.h"
#include "Application.h"
#include "fc/Math.h"
#include <QPainter>
#include <QDebug>
#include <Core.h>

//
#define TAG "FinChartDataCandles"

//
using namespace tmio;

//
FinChartDataCandles::FinChartDataCandles(FinChartWidget* chart) : FinChartData(chart)
{
    _entries=new QVector<Entry>();
    _visibleEntries=new QVector<VisibleEntry>();
    _low=0;
    _high=1;
    _volumeLow=0;
    _volumeHigh=1;
    _updated=false;
    _interval=60;
    _currentTimeX=0;
    _candleWidth=6;
    _candleSpacing=3;
    _indicatorLow=0;
    _indicatorHigh=1;
    _currentTime                =QDateTime::fromSecsSinceEpoch(0);
    _bollingerBandsEnabled      =false;
    _bollingerBandsHistory      =20;
    _bollingerBandsMultiplier   =2;
    _SMAEnabled[0]=_SMAEnabled[1]=_SMAEnabled[2]=false;
    _SMAHistory[0]=10;
    _SMAHistory[1]=20;
    _SMAHistory[2]=200;
    _macdShortHistory=12;
    _macdLongHistory=26;
    _macdSignalHistory=9;
    _VMAEnabled[0]=_VMAEnabled[1]=false;
    _VMAHistory[0]=10;
    _VMAHistory[1]=100;
}

//
FinChartDataCandles::~FinChartDataCandles()
{
    delete(_entries);
    delete(_visibleEntries);
}

//
void                        FinChartDataCandles::update(bool invalidate)
{
    if( (!_updated)||(invalidate) )
    {
        QDateTime   chartStartTime=_chart->startTime();
        int         extendVisibleEntries=365;
        QVector<double>         closes;
        QVector<double>         macd;
        QVector<double>         lows;
        QVector<double>         highs;
        QVector<double>         volumes;
        QVector<VisibleEntry>   ves;
        QVector<double>         macdEmaShort;
        QVector<double>         macdEmaLong;


        for(int i=0;i<_entries->size();i++)
        {
            const FinChartDataCandles::Entry& e=_entries->at(i);
            closes.append(e.close);
            lows.append(e.low);
            highs.append(e.high);
            volumes.append(e.volume);
            //volumes.append(1.0);
            if(i==0)
            {
                _startTime=e.startTime;
            }
            if(i==1)
            {
                //_interval=_entries->at(0).startTime.msecsTo(e.startTime)/1000;
                chartStartTime=_chart->startTime().addSecs(-_interval*extendVisibleEntries);
            }
            // calc indicators
            /*if( (e.startTime>=chartStartTime)&&
                (e.startTime<=_chart->endTime()) )*/
            {
                VisibleEntry    ve(e);
                // SMA (7, 25, 99)
                ve.sma[0]   =fc::Math::SMA(closes,_SMAHistory[0]);
                ve.sma[1]   =fc::Math::SMA(closes,_SMAHistory[1]);
                ve.sma[2]   =fc::Math::SMA(closes,_SMAHistory[2]);
                // VWMA
                ve.vwma[0]  =fc::Math::VWMA(closes,volumes,_SMAHistory[0]);
                ve.vwma[1]  =fc::Math::VWMA(closes,volumes,_SMAHistory[1]);
                ve.vwma[2]  =fc::Math::VWMA(closes,volumes,_SMAHistory[2]);
                // VMA
                ve.vma[0]   =fc::Math::SMA(volumes,_VMAHistory[0]);
                ve.vma[1]   =fc::Math::SMA(volumes,_VMAHistory[1]);
                // MACD
                // https://medium.com/stature-network/understanding-cryptocurrency-trading-signals-macd-in-300-words-b962bc100245
                // VW: https://www.tradingview.com/script/fhPYhWYA-Indicators-Volume-Weighted-MACD-Histogram-Sentiment-Zone-Osc/
                {
                    double s,l;
                    s=  fc::Math::EMA(closes,_macdShortHistory,macdEmaShort);
                    l=  fc::Math::EMA(closes,_macdLongHistory,macdEmaLong);
                    ve.macd=s-l;
                    macd.append(ve.macd);
                    ve.signal=fc::Math::SMA(macd,_macdSignalHistory);
                }
                // BOLLINGER BANDS
                ve.bollingerAvg =fc::Math::SMA(closes,_bollingerBandsHistory);
                ve.bollingerDev =fc::Math::DEV(closes,_bollingerBandsHistory)*(_bollingerBandsMultiplier);
                ves.append(ve);
            }
            _endTime=e.startTime;
            // fast forward optimization (TODO: fast forward, not as critical)
            if(i==2)
            {
                int skipTime=_chart->startTime().toSecsSinceEpoch()-e.startTime.toSecsSinceEpoch();
                int intskipStep=(skipTime/_interval)-extendVisibleEntries;
                //EG_TRACE(TAG,QString("Skip steps: %1").arg(intskipStep));
                if(intskipStep>0) {
                    i+=intskipStep;
                    ves.clear();
                    closes.clear();
                    macd.clear();
                    lows.clear();
                    highs.clear();
                    volumes.clear();
                    macdEmaShort.clear();
                    macdEmaLong.clear();
                }
            }
        }
        // build visible entries % bounds
        _visibleEntries->clear();
        for(int i=0;i<ves.size();i++)
        {
            // TODO: clip not visible entries
            const FinChartDataCandles::VisibleEntry& ve=ves.at(i);
            const FinChartDataCandles::Entry& e=ve.e;
            if( (e.startTime>=_chart->startTime())&&
                (e.startTime<=_chart->endTime()) )
            {
                if(_visibleEntries->length()==0)
                {
                    _low=e.low;
                    _high=e.high;
                    _volumeHigh=e.volume;
                    _volumeLow=e.volume;
                    _indicatorLow=ve.macd;
                    _indicatorHigh=ve.macd;
                }
                _low=qMin(_low,e.low);
                _high=qMax(_high,e.high);
                _low=qMin(_low,ve.bollingerAvg-ve.bollingerDev);
                _high=qMax(_high,ve.bollingerAvg+ve.bollingerDev);
                for(int ima=0;ima<3;ima++)
                {
                    if(ve.sma[ima]>0)
                    {
                        _low=qMin(_low,ve.sma[ima]);
                        _high=qMax(_high,ve.sma[ima]);
                    }
                    if(ve.vwma[ima]>0)
                    {
                        _low=qMin(_low,ve.vwma[ima]);
                        _high=qMax(_high,ve.vwma[ima]);
                    }
                }
                // macd & signal
                _indicatorLow=qMin(_indicatorLow,ve.macd);
                _indicatorHigh=qMax(_indicatorHigh,ve.macd);
                _indicatorLow=qMin(_indicatorLow,ve.signal);
                _indicatorHigh=qMax(_indicatorHigh,ve.signal);
                _indicatorLow=qMin(_indicatorLow,ve.macd-ve.signal);
                _indicatorHigh=qMax(_indicatorHigh,ve.macd-ve.signal);
                // volume
                _volumeLow=qMin(_volumeLow,e.volume);
                _volumeHigh=qMax(_volumeHigh,e.volume);
                //_indicatorLow=qMin(_indicatorLow,ve.ema);
                //_indicatorHigh=qMax(_indicatorHigh,ve.ema);
                if( (e.startTime>=_chart->startTime().addSecs(-_interval))&&
                    (e.startTime<=_chart->endTime().addSecs(+_interval)) )
                {
                    _visibleEntries->append(ve);
                }
            }
        }
        if(qAbs(_volumeHigh-_volumeLow)<=std::numeric_limits<double>::epsilon())
        {
            _volumeHigh+=std::numeric_limits<double>::epsilon()*1000;
        }
        if(qAbs(_high-_low)<=std::numeric_limits<double>::epsilon())
        {
            _low-=std::numeric_limits<double>::epsilon()*1000;
            _high+=std::numeric_limits<double>::epsilon()*1000;
        }
        _low-=(_high-_low)*0.1;
        _high+=(_high-_low)*0.1;
        //Q_ASSERT(qAbs(_high-_low)>std::numeric_limits<double>::epsilon());
        if(qAbs(_indicatorHigh-_indicatorLow)<=std::numeric_limits<double>::epsilon())
        {
            _indicatorLow-=std::numeric_limits<double>::epsilon()*1000;
            _indicatorHigh+=std::numeric_limits<double>::epsilon()*1000;
        }
        _indicatorLow-=(_indicatorHigh-_indicatorLow)*0.1;
        _indicatorHigh+=(_indicatorHigh-_indicatorLow)*0.1;

        _updated=true;
    }
}

//
QDateTime                   FinChartDataCandles::startTime()
{
    update(false);
    return(_startTime);
}

//
QDateTime                   FinChartDataCandles::endTime()
{
    update(false);
    return(_endTime);
}

//
bool                        FinChartDataCandles::currentTime(QDateTime& t,int& snapX)
{
    t=_currentTime;
    snapX=_currentTimeX;
    return(true);
}

//
int                         FinChartDataCandles::interval()
{
    update(false);
    return(_interval);
}

//
int                         FinChartDataCandles::minIntervalWidth()
{
    update(false);
    return(_candleWidth+_candleSpacing);
}

//
bool                        FinChartDataCandles::getLimits(double& low,double& high,double& last)
{
    if(_visibleEntries->length())
    {
        const Entry& e=_visibleEntries->last().e;
        low=_low;
        high=_high;
        last=e.close*((e.close<e.open)?-1:1);
        return(true);
    }
    return(false);
}

//
bool                        FinChartDataCandles::getVolumeLimits(double& low,double& high,double& last)
{
    if(_visibleEntries->length())
    {
        const Entry& e=_visibleEntries->last().e;
        low=0;//_volumeLow;
        high=_volumeHigh;
        last=e.volume*((e.close<e.open)?-1:1);
        return(true);
    }
    return(false);
}

//
bool                        FinChartDataCandles::getIndicatorLimits(double& low,double& high,double& last)
{
    if(_visibleEntries->length())
    {
        const VisibleEntry& ve=_visibleEntries->last();
        low=_indicatorLow;
        high=_indicatorHigh;
        //last=low;//e.volume*((e.close<e.open)?-1:1);
        last=ve.macd-ve.signal;
        return(true);
    }
    return(false);
}

//
void                        FinChartDataCandles::paintEvent(QPaintEvent* /*evt*/)
{
    QPainter painter(_chart);
    QDateTime chartStartTime=_chart->startTime().addSecs(-interval()/2);
    // Update low/high (separete to another function)
    update(false);

    // draw candles
    painter.resetTransform();
    painter.setClipRect(_chart->chartRect().adjusted(0,0,1,1));
    painter.setTransform(QTransform::fromTranslate(_chart->chartRect().left(),_chart->chartRect().top()));
    if(_bollingerBandsEnabled)
    {
        drawBollingerBands(&painter);
    }
    drawMovingAverages(&painter);
    drawCandles(&painter);
    // draw volumes
    painter.resetTransform();
    painter.setClipRect(_chart->volumeRect().adjusted(0,0,1,1));
    painter.setTransform(QTransform::fromTranslate(_chart->volumeRect().left(),_chart->volumeRect().top()));
    drawVolumeMovingAverages(&painter);
    drawVolumes(&painter);
    // draw indicators
    painter.resetTransform();

    painter.setClipRect(_chart->indicatorRect().adjusted(0,0,1,1));
    painter.setTransform(QTransform::fromTranslate(_chart->indicatorRect().left(),_chart->indicatorRect().top()));
    drawIndicators(&painter);
}

//
int                         FinChartDataCandles::entryCount()
{
    return(_entries->length());
}

//
FinChartDataCandles::Entry  FinChartDataCandles::entryAt(int index)
{
    return(_entries->at(index));
}

//
void                        FinChartDataCandles::clear()
{
    _entries->clear();
    _visibleEntries->clear();
    _low=0;
    _high=1;
    _volumeLow=0;
    _volumeHigh=1;
    _updated=false;
    _chart->invalidate();
}

//
void                        FinChartDataCandles::setInterval(int intervalSeconds)
{
    _interval=intervalSeconds;
}

//
void                        FinChartDataCandles::setEntry(const Entry& entry)
{
    bool found=false;
    // try to replace existing entry
    int testParse=0;
    for(int i=0;i<_entries->size();i++)
    {
        testParse++;
        int ri=(_entries->size()-1)-i;
        const Entry&  e=_entries->at(ri);
        if(e.startTime<entry.startTime)
            break;
        if(e.startTime==entry.startTime)
        {
            //e=entry;
            (*_entries)[ri]=entry;
            found=true;
            break;
        }
    }
    if(!found)
    {
        _entries->append(entry);
    }
    _updated=false;
    _chart->invalidate();
}

//
bool                        FinChartDataCandles::isLatestTime()
{
    // check if chart end time is on current candle
    if(_entries->length())
    {
        if(qAbs(_chart->endTime().toSecsSinceEpoch()-_entries->last().startTime.toSecsSinceEpoch())<interval())
        {
            return(true);
        }
    }
    return(false);
}

//
void                        FinChartDataCandles::setBollingerBands(bool enable,int history,double multiplier)
{
    _bollingerBandsEnabled      =enable;
    _bollingerBandsHistory      =history;
    _bollingerBandsMultiplier   =multiplier;
    _updated=false;
    _chart->invalidate();
}

//
void                        FinChartDataCandles::setMovingAverages(int index,bool enable,int history)
{
    _SMAEnabled[index]=enable;
    _SMAHistory[index]=history;
}

//
void                        FinChartDataCandles::setVolumeMovingAverages(int index,bool enable,int history)
{
    _VMAEnabled[index]=enable;
    _VMAHistory[index]=history;
}

//
void                        FinChartDataCandles::setIndicatorMACD(int shortHistory,int longHistory,int signalHistory)
{
    _macdShortHistory=shortHistory;
    _macdLongHistory=longHistory;
    _macdSignalHistory=signalHistory;
}

//
void                        FinChartDataCandles::drawCandles(QPainter* painter)
{
    const ::Style&  style=Application::instance().style();
    int candleExtent=_candleWidth/2;
    double width=static_cast<double>(_chart->chartRect().size().width());
    double height=static_cast<double>(_chart->chartRect().size().height());
    QDateTime chartStartTime=_chart->startTime().addSecs(-interval()/2);

    qint64      startTime=_chart->startTime().toSecsSinceEpoch();
    qint64      endTime=_chart->endTime().toSecsSinceEpoch();
    qint64      cursorTime=_chart->cursorTime().toSecsSinceEpoch();
    double      timeSpan=endTime-startTime;
    bool        selectionFound=false;

    for(int i=0;i<_visibleEntries->size();i++)
    {
        const VisibleEntry& ve=_visibleEntries->at(i);
        const Entry& e=ve.e;
        bool    selectedCandle=false;
        if(!selectionFound)
        {
            if( (cursorTime-e.startTime.toSecsSinceEpoch())<=(interval()/2))
            {
                selectionFound=selectedCandle=true;
            }
        }

        //_chart->startTime().to e.startTime

        //curX=0;
        // calc X
        double t=e.startTime.toSecsSinceEpoch();
        double tx=(t-startTime)/timeSpan;
        int curX=qRound(tx*width);

        //float x=((double)i)/((double)entries.size());
        //float px=x*width;
        const double y0=1-((e.open-_low)/(_high-_low));
        const double y1=1-((e.close-_low)/(_high-_low));
        const double py0=y0*height;
        const double py1=y1*height;

        const double l0=1-((e.high-_low)/(_high-_low));
        const double l1=1-((e.low-_low)/(_high-_low));
        const double ly0=l0*height;
        const double ly1=l1*height;

        //pe.painter->drawLine(QPoint(px,py0)+pe.chartRect.topLeft(),QPoint(px,py1)+pe.chartRect.topLeft());
        //painter.drawLine(QPoint(curX,py0),QPoint(curX,py1));

        if(selectedCandle)
        {
            //painter.drawLine(QPoint(curX,0),QPoint(curX,_chart->chartRect().height()));
            painter->setPen(QColor(0,0,0,0));
            painter->setBrush(QBrush(QColor(0,0,0,32)));
            //painter->drawRect(QRect(QPoint(curX-(candleExtent+1),0),QPoint(curX+(candleExtent+1),height+1)));
            _currentTime=e.startTime;
            _currentTimeX=curX;
        }

        if(e.close>=e.open)
        {
            painter->setPen(QPen(style.color(::Style::colorCandleIncrease),1.));
            painter->setBrush(QBrush(style.color(::Style::colorCandleIncrease)));
        }
        else
        {
            painter->setPen(QPen(style.color(::Style::colorCandleDecrease),1.));
            painter->setBrush(QBrush(style.color(::Style::colorCandleDecrease)));
        }
        painter->drawLine(QPoint(curX,ly0),QPoint(curX,ly1));

        if(ve.tradeSignal)
        {
            painter->setPen( (ve.tradeSignal<0)?QColor(255,0,0):QColor(0,255,0));
            painter->drawLine(QPoint(curX,0),QPoint(curX,height));
            //painter->drawLine();
        }

        painter->setPen(Qt::NoPen);

        if(qRound(py0)!=qRound(py1))
        {
            painter->drawRect(QRect(QPoint(curX-candleExtent,qRound(py0)),QPoint(curX+candleExtent,qRound(py1))));
        }
        else
        {
            painter->drawRect(QRect(QPoint(curX-candleExtent,qRound(py0)),QPoint(curX+candleExtent,qRound(py1+1))));
        }
    }
}

//
void                        FinChartDataCandles::drawMovingAverages(QPainter* painter)
{
    const double width=static_cast<double>(_chart->chartRect().size().width());
    const double height=static_cast<double>(_chart->chartRect().size().height());
    qint64      startTime=_chart->startTime().toSecsSinceEpoch();
    qint64      endTime=_chart->endTime().toSecsSinceEpoch();
    double      timeSpan=endTime-startTime;


    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor colors[3]=
    {
        QColor(192,192,64,192),
        QColor(128,64,192,192),
        QColor(192,64,128,192)
    };
    for(int ima=0;ima<3;ima++)
    {
        if(!_SMAEnabled[ima])
            continue;
        QVector<QPointF>    path;
        QVector<QPointF>    vwpath;
        for(int i=0;i<_visibleEntries->size();i++)
        {
            const VisibleEntry& ve=_visibleEntries->at(i);

            double t=ve.e.startTime.toSecsSinceEpoch();
            double tx=(t-startTime)/timeSpan;
            int px=qRound(tx*width);

            //float x=((double)i)/((double)entries.size());
            //float px=x*width;
            if(ve.sma[ima]>0)
            {
                const double y=1-((ve.sma[ima]-_low)/(_high-_low));
                const double py=y*height;
                path.append(QPointF(px,py));
            }
            if(ve.vwma[ima]>0)
            {
                const double y=1-((ve.vwma[ima]-_low)/(_high-_low));
                const double py=y*height;
                vwpath.append(QPointF(px,py));
            }
        }
        QBrush  brush(colors[ima]);
        painter->setPen(QPen(brush,1.5,Qt::DotLine));
        painter->drawPolyline(vwpath.data(),vwpath.length());
        painter->setPen(QPen(brush,1.5,Qt::SolidLine));
        painter->drawPolyline(path.data(),path.length());
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
}

//
void                        FinChartDataCandles::drawVolumeMovingAverages(QPainter* painter)
{
    const double width=static_cast<double>(_chart->volumeRect().size().width());
    const double height=static_cast<double>(_chart->volumeRect().size().height());
    qint64      startTime=_chart->startTime().toSecsSinceEpoch();
    qint64      endTime=_chart->endTime().toSecsSinceEpoch();
    double      timeSpan=endTime-startTime;


    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor colors[2]=
    {
        QColor(192,192,64,192),
        QColor(192,64,128,192)
    };
    for(int ima=0;ima<2;ima++)
    {
        if(!_VMAEnabled[ima])
            continue;
        QVector<QPointF>    path;
        QVector<QPointF>    vwpath;
        for(int i=0;i<_visibleEntries->size();i++)
        {
            const VisibleEntry& ve=_visibleEntries->at(i);

            double t=ve.e.startTime.toSecsSinceEpoch();
            double tx=(t-startTime)/timeSpan;
            int px=qRound(tx*width);

            //float x=((double)i)/((double)entries.size());
            //float px=x*width;
            if(ve.vma[ima]>0)
            {
                const double y=1-((ve.vma[ima]-_volumeLow)/(_volumeHigh-_volumeLow));
                const double py=y*height;
                path.append(QPointF(px,py));
            }
        }
        QBrush  brush(colors[ima]);
        painter->setPen(QPen(brush,1.5,Qt::SolidLine));
        painter->drawPolyline(path.data(),path.length());
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
}

//
void                        FinChartDataCandles::drawBollingerBands(QPainter* painter)
{
    const double width=static_cast<double>(_chart->chartRect().size().width());
    const double height=static_cast<double>(_chart->chartRect().size().height());
    qint64      startTime=_chart->startTime().toSecsSinceEpoch();
    qint64      endTime=_chart->endTime().toSecsSinceEpoch();
    double      timeSpan=endTime-startTime;


    painter->setRenderHint(QPainter::Antialiasing, true);
    if(_visibleEntries->size()>=2)
    {
        int len=_visibleEntries->size();
        QVector<QPointF>    path[3];
        for(int ibb=0;ibb<3;ibb++)
        {
            for(int i=0;i<len;i++)
            {
                const VisibleEntry& ve=_visibleEntries->at(i);
                double bands[]={ve.bollingerAvg-ve.bollingerDev,ve.bollingerAvg,ve.bollingerAvg+ve.bollingerDev};
                double t=ve.e.startTime.toSecsSinceEpoch();
                double tx=(t-startTime)/timeSpan;
                int px=qRound(tx*width);
                double y=1-((bands[ibb]-_low)/(_high-_low));
                double py=y*height;
                path[ibb].append(QPointF(px,py));
            }
        }
        // build polygons
        double          sepWidth=3;
        QPainterPath    lowerPath, higherPath;
        lowerPath.moveTo(path[0].at(0));
        higherPath.moveTo(path[2].at(0));
        for(int i=1;i<len;i++)
        {
            lowerPath.lineTo(path[0].at(i));
            higherPath.lineTo(path[2].at(i));
        }
        for(int i=0;i<len;i++)
        {
            QPointF     avg=path[1].at(path[1].length()-(i+1));
            lowerPath.lineTo(avg+QPointF(0,sepWidth*0.5));
            higherPath.lineTo(avg-QPointF(0,sepWidth*0.5));
        }
        painter->fillPath(lowerPath,QBrush(QColor(128,0,0,20)));
        painter->fillPath(higherPath,QBrush(QColor(0,128,0,20)));
        // middle line
        QBrush  brush(QColor(0,0,0,40));
        painter->setPen(QPen(brush,sepWidth,Qt::DashLine));
        painter->drawPolyline(path[1].data(),path[1].length());
    }
    painter->setRenderHint(QPainter::Antialiasing, false);
}

//
void                        FinChartDataCandles::drawVolumes(QPainter* painter)
{
    const ::Style&  style=Application::instance().style();
    int candleExtent=_candleWidth/2;
    const double width=static_cast<double>(_chart->volumeRect().size().width());
    const double height=static_cast<double>(_chart->volumeRect().size().height());
    QDateTime chartStartTime=_chart->startTime().addSecs(-interval()/2);

    qint64      startTime=_chart->startTime().toSecsSinceEpoch();
    qint64      endTime=_chart->endTime().toSecsSinceEpoch();
    qint64      cursorTime=_chart->cursorTime().toSecsSinceEpoch();
    double      timeSpan=endTime-startTime;
    bool        selectionFound=false;

    painter->setPen(Qt::NoPen);
    for(int i=0;i<_visibleEntries->size();i++)
    {
        const Entry& e=_visibleEntries->at(i).e;
        bool    selectedCandle=false;
        if(!selectionFound)
        {
            if( (cursorTime-e.startTime.toSecsSinceEpoch())<=(interval()/2))
            {
                selectionFound=selectedCandle=true;
            }
        }

        //_chart->startTime().to e.startTime

        //curX=0;
        // calc X
        double t=e.startTime.toSecsSinceEpoch();
        double tx=(t-startTime)/timeSpan;
        int curX=qRound(tx*width);

        //float x=((double)i)/((double)entries.size());
        //float px=x*width;
        double y0=1-((e.volume)/_volumeHigh);
        double y1=1-((0)/_volumeHigh);
        double py0=y0*height;
        double py1=y1*height;


        //pe.painter->drawLine(QPoint(px,py0)+pe.chartRect.topLeft(),QPoint(px,py1)+pe.chartRect.topLeft());
        //painter.drawLine(QPoint(curX,py0),QPoint(curX,py1));

        if(selectedCandle)
        {
            //painter.drawLine(QPoint(curX,0),QPoint(curX,_chart->chartRect().height()));
            //painter->setBrush(QBrush(QColor(230,230,230)));
            //painter->drawRect(QRect(QPoint(curX-(candleExtent+1),0),QPoint(curX+(candleExtent+1),height+1)));
        }

        if(e.close>=e.open)
        {
            painter->setBrush(QBrush(style.color(::Style::colorCandleIncrease)));
        }
        else
        {
            painter->setBrush(QBrush(style.color(::Style::colorCandleDecrease)));
        }

        painter->drawRect(QRect(QPoint(curX-candleExtent,qRound(py0)),QPoint(curX+candleExtent,qRound(py1))));
    }
}

//
void                        FinChartDataCandles::drawIndicators(QPainter* painter)
{
    int candleExtent=_candleWidth/2;
    const double width=static_cast<double>(_chart->indicatorRect().size().width());
    const double height=static_cast<double>(_chart->indicatorRect().size().height());
    QDateTime chartStartTime=_chart->startTime().addSecs(-interval()/2);

    qint64              startTime=_chart->startTime().toSecsSinceEpoch();
    qint64              endTime=_chart->endTime().toSecsSinceEpoch();
    double              timeSpan=endTime-startTime;

    QVector<QPointF>    macdPath;
    QVector<QPointF>    signalPath;
    painter->setPen(Qt::NoPen);
    for(int i=0;i<_visibleEntries->size();i++)
    {
        const VisibleEntry& ve  =_visibleEntries->at(i);
        const Entry&        e   =ve.e;
        double t=e.startTime.toSecsSinceEpoch();
        double tx=(t-startTime)/timeSpan;
        int curX=qRound(tx*width);

        //float x=((double)i)/((double)entries.size());
        //float px=x*width;
        double macdY=1-((ve.macd-_indicatorLow)/(_indicatorHigh-_indicatorLow));
        double macdPy=macdY*height;

        double signalY=1-((ve.signal-_indicatorLow)/(_indicatorHigh-_indicatorLow));
        double signalPy=signalY*height;

        macdPath.append(QPointF(curX,macdPy));
        signalPath.append(QPointF(curX,signalPy));

        // draw difference
        double zeroY=1-((0-_indicatorLow)/(_indicatorHigh-_indicatorLow));
        double zeroPy=zeroY*height;
        //double buySignalPy=signalPy-macdPy;
        if(ve.macd>=ve.signal)
        {
            double buy=ve.macd-ve.signal;
            double buyY=1-((buy-_indicatorLow)/(_indicatorHigh-_indicatorLow));
            double buyPy=buyY*height;
            painter->setBrush(QBrush(QColor(0,192,0)));
            if(qRound(zeroPy)==qRound(buyPy))
                buyPy+=1;
            painter->drawRect(QRect(QPoint(curX-candleExtent,qRound(zeroPy)),QPoint(curX+candleExtent,qRound(buyPy))));
        }
        else
        {
            double buy=ve.macd-ve.signal;
            double buyY=1-((buy-_indicatorLow)/(_indicatorHigh-_indicatorLow));
            double buyPy=buyY*height;
            painter->setBrush(QBrush(QColor(192,0,0)));
            if(qRound(zeroPy)==qRound(buyPy))
                buyPy-=1;
            painter->drawRect(QRect(QPoint(curX-candleExtent,qRound(zeroPy)),QPoint(curX+candleExtent,qRound(buyPy))));
        }
    }
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QColor(0,0,255));
    painter->drawPolyline(macdPath.data(),macdPath.length());
    painter->setPen(QColor(128,128,0));
    painter->drawPolyline(signalPath.data(),signalPath.length());
    painter->setRenderHint(QPainter::Antialiasing, false);
}

