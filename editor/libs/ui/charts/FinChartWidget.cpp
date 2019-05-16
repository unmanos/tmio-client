#include "FinChartWidget.h"
#include "FinChartData.h"
#include "FinChartDataCandles.h"
#include "Application.h"
#include <Core.h>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyle>
#include <QMovie>
#include <QToolButton>
#include <QAction>
#include <QDebug>
#include <QTimeZone>
#include <math.h>

//
#define TAG "finchart.FinChartWidget"

//
FinChartWidget::FinChartWidget(QWidget *parent) : QWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onStep()));

    _needRedraw=false;
    _updated=false;
    _dragMode=dragNone;
    _cursorX=-1;
    _cursorY=-1;
    _gripHeight=0;

    timer->start(1000/60/*fps*/);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    QSize       defaultSize(640,480);
    updateLayouts(&defaultSize);
}

//
FinChartWidget::~FinChartWidget()
{
    QList<FinChartData*>::iterator i;
    for (i = _datas.begin(); i != _datas.end(); ++i)
    {
        delete(*i);
    }
}

//
FinChartDataCandles*  FinChartWidget::addCandles()
{
    FinChartDataCandles* data=new FinChartDataCandles(this);
    _datas.append(data);
    return(data);
}

//
void FinChartWidget::invalidate()
{
    _needRedraw=true;
    _updated=false;
}

//
QRect FinChartWidget::chartRect()
{
    return(_chartRect);
}

//
QRect FinChartWidget::volumeRect()
{
    return(_volumeRect);
}

//
QRect FinChartWidget::indicatorRect()
{
    return(_indicatorRect);
}

//
QDateTime FinChartWidget::startTime()
{
    return(_startTime);
}

//
QDateTime FinChartWidget::endTime()
{
    return(_endTime);
}

//
QDateTime FinChartWidget::cursorTime()
{
    return(_cursorTime);
}

//
void FinChartWidget::updateTimeRange(bool latest)
{
    // update all market data
    QList<FinChartData*>::iterator i;
    double      chartWidth=chartRect().width();
    double      intervalWidth=1;
    double      interval=0;
    QDateTime   lastMarketTime=QDateTime::fromSecsSinceEpoch(0);
    for (i = _datas.begin(); i != _datas.end(); ++i)
    {
        (*i)->update(false);
        intervalWidth=qMax((*i)->minIntervalWidth(),(int)intervalWidth);
        interval=(interval==0)?(*i)->interval():qMin((*i)->interval(),(int)interval);
        //lastMarketTime=qMax((*i)->endTime(),lastMarketTime);
        lastMarketTime=(*i)->endTime();
    }
    if(latest)
    {
        _endTime     =lastMarketTime.addSecs(interval/2);//QDateTime::currentDateTime();
    }
    double       spanSeconds =(chartWidth/intervalWidth)*interval /*add one interval for clipping*/;
    _startTime   =_endTime.addMSecs(-spanSeconds*1000.);
    //updateChart(true);
    _updated=false;
}

//
void FinChartWidget::updateChart(bool invalidate)
{
    if(invalidate)
    {
        _updated=false;
    }
    if(!_updated)
    {
        // update all market data
        //setTime(QDateTime::currentDateTime());

        QList<FinChartData*>::iterator i;
        for (i = _datas.begin(); i != _datas.end(); ++i)
        {
            //(*i)->update(true);
            (*i)->update(!_updated);
        }

        _updated=true;
    }
}

//
void FinChartWidget::updateLayouts(const QSize* size)
{
    const QSize     layoutSize=size?(*size):QSize(width(),height());
    const Style&    s=_style;
    int totalMarginY=s.marginTop+s.marginBottom;
    _chartRect  =QRect(
                s.marginX,
                s.marginTop,
                layoutSize.width()-(s.marginX*2+s.scaleWidth),
                layoutSize.height()-(totalMarginY+s.separatorHeight*2+s.volumeHeight+s.indicatorHeight));
    _volumeRect =QRect(
                s.marginX,
                _chartRect.bottom()+s.separatorHeight,
                layoutSize.width()-(s.marginX*2+s.scaleWidth),
                s.volumeHeight);
    _indicatorRect =QRect(
                s.marginX,
                _volumeRect.bottom()+s.separatorHeight,
                layoutSize.width()-(s.marginX*2+s.scaleWidth),
                s.indicatorHeight);

    invalidate();
}

//
void FinChartWidget::paintSticker(QPainter* painter,QColor color,QPoint p,Qt::Orientation orientation,QString text)
{
    //return;
    //text="fuck";

    const ::Style&  style=Application::instance().style();
    int             textMargin=4;
    QFontMetrics    fm(painter->font(), painter->device());
    QRect           boundingRect    =fm.boundingRect(text);
    int             textHeight      =boundingRect.height();
    int             halfTextHeight  =(textHeight/2)+2;


    int nPoint=0;
    QPoint sticker[16];
    QPoint textPos;
    painter->setPen(QColor(color.red(),color.green(),color.blue(),128));
    painter->setBrush(style.palette().color(QPalette::Base));
    if(orientation==Qt::Horizontal)
    {
        sticker[0]=p;
        sticker[1]=sticker[0]+QPoint(textMargin,-halfTextHeight);
        sticker[2]=sticker[1]+QPoint(boundingRect.width()+textMargin*2,0);
        sticker[3]=sticker[2]+QPoint(0,halfTextHeight*2);
        sticker[4]=sticker[3]+QPoint(-(boundingRect.width()+textMargin*2),0);
        sticker[5]=sticker[0];
        nPoint=5;
        textPos=QPoint(p.x()+textMargin*2,p.y()+(textHeight/2-2) );
    }
    if(orientation==Qt::Vertical)
    {
        sticker[0]=p;
        sticker[1]=sticker[0]+QPoint(+halfTextHeight,textMargin);
        sticker[2]=sticker[1]+QPoint( (boundingRect.width()/2+textMargin)-halfTextHeight,0);
        sticker[3]=sticker[2]+QPoint(0,halfTextHeight*2);
        sticker[4]=sticker[3]+QPoint(-(boundingRect.width()+textMargin*2),0);
        sticker[5]=sticker[4]+QPoint(0,-halfTextHeight*2);
        sticker[6]=sticker[5]+QPoint( (boundingRect.width()/2+textMargin)-halfTextHeight,0);
        sticker[7]=sticker[0];
        // calc decal
        int decalX=0;
        int maxDecal=(sticker[6].x()-sticker[5].x());
        if(sticker[4].x()<=chartRect().left())
        {
            decalX=qMin(chartRect().left()-sticker[4].x(),maxDecal);
        }
        if(sticker[3].x()>=chartRect().right())
        {
            decalX=-qMin(sticker[3].x()-chartRect().right(),maxDecal);
        }

        sticker[2].rx()+=decalX;
        sticker[3].rx()+=decalX;
        sticker[4].rx()+=decalX;
        sticker[5].rx()+=decalX;

        nPoint=8;
        textPos=QPoint(sticker[4].x()+textMargin,p.y()+(textHeight+2) );
    }
    painter->drawConvexPolygon(sticker, nPoint);
    painter->setPen(style.palette().color(QPalette::Text));
    painter->drawText(textPos,text);
}

//
void FinChartWidget::paintPriceScale(QPainter* painter,double low,double current,double high)
{
    const ::Style&  style=Application::instance().style();
    QColor          textColor=style.palette().color(QPalette::Text);
    double          span=(high-low);
    double          gridStep=0.00000001;
    double          nStep=0;
    int             textMargin=4;
    bool            priceDrop=false;
    int             perfectCount=chartRect().height()/40;
    if(current<0)
    {
        current*=-1;
        priceDrop=true;
    }

    do
    {
        if(nStep!=0)
        {
            gridStep*=10;
        }
        nStep=span/gridStep;
    }
    while(nStep>perfectCount);
    if(nStep<2)
    {
        gridStep/=5;
        nStep=span/gridStep;
    }
    else if(nStep<5)
    {
        gridStep/=2;
        nStep=span/gridStep;
    }

    //_low%gridStep;
    //EG_TRACE(TAG,QString("Span: %1 (%2)").arg(gridStep).arg(nStep));
    double      gridStart=low-std::remainder(low,gridStep);
    double      gridCurrent=gridStart;
    int         decimals=(((int)current)<1)?8:2;
    QFontMetrics fm(painter->font(), painter->device());
    QRect       boundingRect=fm.boundingRect(QString::number(current,'f',decimals));
    int         textHeight = boundingRect.height();

    painter->translate(chartRect().topLeft());
    while(gridCurrent<high)
    {
        //EG_TRACE(TAG,QString("Step: %1").arg(gridCurrent));

        float y=1-((gridCurrent-low)/(high-low));
        float py=y*((double)chartRect().height());

        if( (py>textHeight)&&(py<(chartRect().height()-textHeight)) )
        {
            painter->setPen(QColor(textColor.red(),textColor.green(),textColor.blue(),32));
            painter->drawLine(QPoint(0,py),QPoint(chartRect().width(),py));
            painter->setPen(QColor(textColor.red(),textColor.green(),textColor.blue(),64));
            painter->drawText(QPoint(chartRect().width()+textMargin*2,py),QString::number(gridCurrent,'f',decimals));
        }

        gridCurrent+=gridStep;
    }
    // draw current price
    {
        float cy=1-((current-low)/(high-low));
        float cpy=cy*((double)chartRect().height());
        paintSticker(painter,priceDrop?style.color(::Style::colorTextChangeDecrease):style.color(::Style::colorTextChangeIncrease),QPoint(chartRect().width(),cpy),Qt::Horizontal,QString::number(current,'f',decimals));
    }
    // draw cursor price
    if( (_cursorY>=chartRect().top())&&(_cursorY<=chartRect().bottom()) )
    {
        double py=(_cursorY-chartRect().top());
        double cy=1-(py/((double)chartRect().height()));
        double price=low+(high-low)*cy;
        painter->setPen(QColor(0,0,255,128));
        painter->drawLine(QPoint(0,py),QPoint(chartRect().width(),py));
        paintSticker(painter,QColor(0,0,255),QPoint(chartRect().width(),py),Qt::Horizontal,QString::number(price,'f',decimals));
    }
    // timeline
    {
        //painter->setPen(QColor(0,0,255,128));
        //painter->drawLine(QPoint(_cursorX-chartRect().left(),0),QPoint(_cursorX-chartRect().left(),chartRect().height()));
    }

    painter->translate(0,0);
    painter->resetTransform();
}

//
void FinChartWidget::paintVolumeScale(QPainter* painter,double low,double current,double high)
{
    const ::Style&  style=Application::instance().style();
    QColor          textColor=style.palette().color(QPalette::Text);
    double          span=(high-low);
    double          gridStep=0.00000001;
    double          nStep=0;
    int             textMargin=4;
    bool            priceDrop=false;
    int             perfectCount=volumeRect().height()/40;
    if(current<0)
    {
        current*=-1;
        priceDrop=true;
    }

    do
    {
        if(nStep!=0)
        {
            gridStep*=10;
        }
        nStep=span/gridStep;
    }
    while(nStep>perfectCount);
    if(nStep<2)
    {
        gridStep/=5;
        nStep=span/gridStep;
    }
    else if(nStep<4)
    {
        gridStep/=2;
        nStep=span/gridStep;
    }

    //_low%gridStep;
    //EG_TRACE(TAG,QString("Span: %1 (%2)").arg(gridStep).arg(nStep));

    double      gridStart=low-std::remainder(low,gridStep);
    double      gridCurrent=gridStart;
    int         decimals=(((int)current)<1)?8:2;
    QFontMetrics fm(painter->font(), painter->device());
    QRect       boundingRect=fm.boundingRect(QString::number(current,'f',decimals));
    int         textHeight = boundingRect.height();

    painter->translate(volumeRect().topLeft());
    while(gridCurrent<high)
    {
        //EG_TRACE(TAG,QString("Step: %1").arg(gridCurrent));

        float y=1-((gridCurrent-low)/(high-low));
        float py=y*((double)volumeRect().height());

        if( (py>textHeight)&&(py<(volumeRect().height()-textHeight)) )
        {
            painter->setPen(QColor(textColor.red(),textColor.green(),textColor.blue(),32));
            painter->drawLine(QPoint(0,py),QPoint(volumeRect().width(),py));
            painter->setPen(QColor(textColor.red(),textColor.green(),textColor.blue(),64));
            painter->drawText(QPoint(volumeRect().width()+textMargin*2,py),QString::number(gridCurrent,'f',decimals));
        }

        gridCurrent+=gridStep;
    }
    // draw current volume
    {
        float cy=1-((current-low)/(high-low));
        float cpy=cy*((double)volumeRect().height());
        paintSticker(painter,priceDrop?style.color(::Style::colorTextChangeDecrease):style.color(::Style::colorTextChangeIncrease),QPoint(chartRect().width(),cpy),Qt::Horizontal,QString::number(current,'f',decimals));
    }
    // draw cursor volume
    if( (_cursorY>=volumeRect().top())&&(_cursorY<=volumeRect().bottom()) )
    {
        double py=(_cursorY-volumeRect().top());
        double cy=1-(py/((double)volumeRect().height()));
        double volume=low+(high-low)*cy;
        painter->setPen(QColor(0,0,255,128));
        painter->drawLine(QPoint(0,py),QPoint(volumeRect().width(),py));
        paintSticker(painter,QColor(0,0,255),QPoint(volumeRect().width(),py),Qt::Horizontal,QString::number(volume,'f',decimals));
    }
    painter->translate(0,0);
    painter->resetTransform();
}

//
void FinChartWidget::paintIndicatorScale(QPainter* painter,double low,double current,double high)
{
    const ::Style&  style=Application::instance().style();
    double          span=(high-low);
    double          gridStep=0.00000001;
    double          nStep=0;
    int             perfectCount=indicatorRect().height()/40;
    do
    {
        if(nStep!=0)
        {
            gridStep*=10;
        }
        nStep=span/gridStep;
    }
    while(nStep>perfectCount);
    if(nStep<2)
    {
        gridStep/=5;
        nStep=span/gridStep;
    }
    else if(nStep<5)
    {
        gridStep/=2;
        nStep=span/gridStep;
    }

    //_low%gridStep;
    //EG_TRACE(TAG,QString("Span: %1 (%2)").arg(gridStep).arg(nStep));

    double      gridStart=low-fmod(low,gridStep);
    double      gridCurrent=gridStart;
    int         decimals=8;
    QFontMetrics fm(painter->font(), painter->device());

    painter->translate(indicatorRect().topLeft());
    while(gridCurrent<high)
    {
        gridCurrent+=gridStep;
    }
    // draw current price
    {
        float cy=1-((current-low)/(high-low));
        float cpy=cy*((double)indicatorRect().height());
        paintSticker(painter,(current<0)?style.color(::Style::colorTextChangeDecrease):style.color(::Style::colorTextChangeIncrease),QPoint(indicatorRect().width(),cpy),Qt::Horizontal,QString::number(current,'f',decimals));
    }
    // draw cursor price
    if( (_cursorY>=indicatorRect().top())&&(_cursorY<=indicatorRect().bottom()) )
    {
        double py=(_cursorY-indicatorRect().top());
        double cy=1-(py/((double)indicatorRect().height()));
        double price=low+(high-low)*cy;
        painter->setPen(QColor(0,0,255,128));
        painter->drawLine(QPoint(0,py),QPoint(indicatorRect().width(),py));
        paintSticker(painter,QColor(0,0,255),QPoint(indicatorRect().width(),py),Qt::Horizontal,QString::number(price,'f',decimals));
    }
    // timeline
    {
        //painter->setPen(QColor(0,0,255,128));
        //painter->drawLine(QPoint(_cursorX-chartRect().left(),0),QPoint(_cursorX-chartRect().left(),chartRect().height()));
    }

    painter->translate(0,0);
    painter->resetTransform();
}

//
static QString  _formatDT(qint64 interval,QDateTime dt)
{
    const qint64    offsetUTC   =QTimeZone::systemTimeZone().offsetFromUtc(dt);
    qint64          epoch       =dt.toSecsSinceEpoch()+offsetUTC;
    if( ( (epoch%(1440*60))==0)||(interval>=1440) )
    {
        return QLocale::system().toString(dt.date(), QLocale::ShortFormat);
    }
    else
    {
        return QLocale::system().toString(dt.time(), QLocale::ShortFormat);
    }
}

//
void FinChartWidget::paintTimeScale(QPainter* painter)
{
    const ::Style&  style       =Application::instance().style();
    const QColor    textColor   =style.palette().color(QPalette::Text);
    // time scale
    QList<qint64>           intervalSets;
    intervalSets.append(360*1440);
    intervalSets.append(180*1440);
    intervalSets.append(120*1440);
    intervalSets.append(90*1440);
    intervalSets.append(60*1440);
    intervalSets.append(30*1440);
    intervalSets.append(7*1440);
    intervalSets.append(6*1440);
    intervalSets.append(5*1440);
    intervalSets.append(4*1440);
    intervalSets.append(3*1440);
    intervalSets.append(2*1440);
    intervalSets.append(1440);
    intervalSets.append(720);
    intervalSets.append(480);
    intervalSets.append(360);
    intervalSets.append(240);
    intervalSets.append(180);
    intervalSets.append(120);
    intervalSets.append(60);
    intervalSets.append(30);
    intervalSets.append(20);
    intervalSets.append(15);
    intervalSets.append(10);
    intervalSets.append(5);

    qint64                  scaleInterval=intervalSets.first();
    foreach(qint64 s,intervalSets)
    {
        double totalTime=_startTime.secsTo(_endTime)/60;
        //double count=((double)intervalSets[intervalMode])/((double)width);
        double count    =totalTime/static_cast<double>(s);
        if(count>10)
        {
            break;
        }
        scaleInterval=s;
    }

    /*QDateTime local(QDateTime::currentDateTime());
    local.secsTo(local.toUTC());
    const qint64    offsetUTC=local.secsTo(local.toUTC());*/

    const int                       width       =chartRect().width();
    qint64                          tickEpoch   =startTime().toSecsSinceEpoch();
    const qint64                    endT        =endTime().toSecsSinceEpoch()+scaleInterval*60L;

    /*tickT+=offsetUTC;
    tickT-=tickT%(scaleInterval*60L);
    tickT-=offsetUTC;*/
    int parsed=0;
    while(tickEpoch<endT)
    {
        const qint64    offsetUTC   =QTimeZone::systemTimeZone().offsetFromUtc(QDateTime::fromSecsSinceEpoch(tickEpoch));
        qint64          tickT       =tickEpoch+offsetUTC;
        tickT=tickT-(tickT%(scaleInterval*60L));
        tickT-=offsetUTC;

        //QDateTime tickT             =QDateTime::fromSecsSinceEpoch(tickEpoch);
        //int         offsetFromUtc
        //qint64      ajustedTickT    =


        double      startT          =(double)(_startTime.toSecsSinceEpoch());
        double      endT            =(double)(_endTime.toSecsSinceEpoch());
        double      t               =(double)tickT;
        double      x               =qRound(((t-startT)/(endT-startT))*width);
        if( (x>=0)&&(x<width) )
        {
            painter->setPen(QColor(textColor.red(),textColor.green(),textColor.blue(),32));
            painter->drawLine(QPoint(x+chartRect().left(),chartRect().top()),QPoint(x+chartRect().left(),chartRect().bottom()+1));
            painter->drawLine(QPoint(x+volumeRect().left(),volumeRect().top()),QPoint(x+volumeRect().left(),volumeRect().bottom()+1));
            painter->drawLine(QPoint(x+indicatorRect().left(),indicatorRect().top()),QPoint(x+indicatorRect().left(),indicatorRect().bottom()+1));

            painter->setPen(QColor(textColor.red(),textColor.green(),textColor.blue(),64));

            const QDateTime tickDT=QDateTime::fromSecsSinceEpoch(tickT);
            QString         text=_formatDT(scaleInterval,tickDT);
            QFontMetrics    fm(painter->font(), painter->device());
            QRect           boundingRect    =fm.boundingRect(text);
            double          textX           =(indicatorRect().left()+x)-boundingRect.width()/2;
            double          textY           =indicatorRect().bottom()+boundingRect.height()+4;
            painter->drawText(QPoint(textX,textY),text);
        }

        tickEpoch+=scaleInterval*60L;
        parsed++;

        //_startTime.
    }
    //TM_TRACE(TAG,QString("Scale time interval %1, parsed %2").arg(scaleInterval).arg(parsed));
    // current time
    {
        QDateTime                       currentTime=QDateTime::fromSecsSinceEpoch(0);
        int                             currentTimeX=0;
        bool                            haveTime=false;
        QList<FinChartData*>::iterator  i;
        for(i = _datas.begin(); i != _datas.end(); ++i)
        {
            if(!haveTime)
            {
                haveTime=(*i)->currentTime(currentTime,currentTimeX);
            }
        }
        if(currentTime!=QDateTime::fromSecsSinceEpoch(0))
        {
            int         chartX  =currentTimeX;
            painter->setPen(QColor(0,0,255,128));
            painter->drawLine(QPoint(chartX+chartRect().left(),chartRect().top()),QPoint(chartX+chartRect().left(),chartRect().bottom()+1));
            painter->drawLine(QPoint(chartX+volumeRect().left(),volumeRect().top()),QPoint(chartX+volumeRect().left(),volumeRect().bottom()+1));
            painter->drawLine(QPoint(chartX+indicatorRect().left(),indicatorRect().top()),QPoint(chartX+indicatorRect().left(),indicatorRect().bottom()+1));
            QDateTime         time=currentTime.toLocalTime();

            QString         text;
            if(currentTime.date()==QDateTime::currentDateTime().date())
            {
                text=QLocale::system().toString(currentTime.toLocalTime().time(),QLocale::ShortFormat);
            }
            else
            {
                text=QLocale::system().toString(currentTime.toLocalTime(),QLocale::ShortFormat);
            }
            paintSticker(painter,QColor(0,0,255),QPoint(chartX+indicatorRect().left(),indicatorRect().bottom()+2),Qt::Vertical,text);
        }
    }
}

//
void FinChartWidget::paintEvent(QPaintEvent* evt)
{
    const ::Style&      style=Application::instance().style();
    //EG_TRACE(TAG, QString("Paint length=%1").arg(_length));
    QWidget::paintEvent(evt);
    QPainter painter(this);
    QList<FinChartData*>::iterator i;

    updateChart(false);

    //painter.drawRect(1,1, width()-2, height()-2);

    painter.setPen(style.palette().color(QPalette::Window));
    painter.setBrush(QBrush(style.palette().color(QPalette::Base)));
    painter.drawRect(chartRect());
    painter.drawRect(volumeRect());
    painter.drawRect(indicatorRect());

    double high=0,low=0,last=0;
    double volumeHigh=0,volumeLow=0,volumeLast=0;
    double indicatorHigh=0,indicatorLow=0,indicatorLast=0;
    bool haveLimits=false;
    bool haveVolumeLimits=false;
    bool haveIndicatorLimits=false;
    for (i = _datas.begin(); i != _datas.end(); ++i)
    {
        (*i)->paintEvent(evt);
        double l,h;
        if((*i)->getLimits(l,h,last))
        {
            if(!haveLimits)
            {
                low=l;
                high=h;
                haveLimits=true;
            }
            high=qMax(high,h);
            low=qMin(low,l);
        }
        if((*i)->getVolumeLimits(l,h,volumeLast))
        {
            if(!haveVolumeLimits)
            {
                volumeLow=l;
                volumeHigh=h;
                haveVolumeLimits=true;
            }
            volumeHigh=qMax(volumeHigh,h);
            volumeLow=qMin(volumeLow,l);
        }
        if((*i)->getIndicatorLimits(l,h,indicatorLast))
        {
            if(!haveIndicatorLimits)
            {
                //TM_TRACE(TAG,QString("%1 - %2").arg(l).arg(h));
                indicatorLow=l;
                indicatorHigh=h;
                haveIndicatorLimits=true;
            }
            indicatorHigh=qMax(indicatorHigh,h);
            indicatorLow=qMin(indicatorLow,l);
        }
        //if((*i)->getL(l,h))
    }
    //
    QPainter painter2(this);
    if(haveLimits)
    {
        if(qAbs(high-low)>std::numeric_limits<double>::epsilon())
            paintPriceScale(&painter2,low,last,high);
    }
    if(haveVolumeLimits)
    {
        if(qAbs(volumeHigh-volumeLow)>std::numeric_limits<double>::epsilon())
            paintVolumeScale(&painter2,volumeLow,volumeLast,volumeHigh);
    }
    if(haveIndicatorLimits)
    {
        //TM_TRACE(TAG,QString("%1 - %2").arg(QString::number(indicatorLow,'f',20)).arg(QString::number(indicatorHigh,'f',20)));
        //Q_ASSERT( (indicatorHigh-indicatorLow)>std::numeric_limits<double>::epsilon());
        if((indicatorHigh-indicatorLow)>std::numeric_limits<double>::epsilon())
        {
            paintIndicatorScale(&painter2,indicatorLow,indicatorLast,indicatorHigh);
        }
    }
    paintTimeScale(&painter2);

    //paintTimeScale(&painter2,volumeLow,volumeLast,volumeHigh);
    //painter2.drawLine(0,0,width(),height());
    painter2.setPen(QColor(0,0,0,32));
    painter2.setBrush(QBrush(QColor(255,255,255,0)));
    painter2.drawRect(chartRect().adjusted(-1,-1,1,1));
    painter2.drawRect(volumeRect().adjusted(-1,-1,1,1));
    painter2.drawRect(indicatorRect().adjusted(-1,-1,1,1));
}

//
void FinChartWidget::wheelEvent(QWheelEvent* event)
{
    if(event->delta()>0) {
    }
    if(event->delta()<0) {
    }
}

//
void FinChartWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if(event->button()==Qt::LeftButton)
    {
        QRect volumeUpRect(chartRect().left(),chartRect().bottom(),chartRect().width(),_style.separatorHeight);
        QRect volumeDownRect(chartRect().left(),volumeRect().bottom(),chartRect().width(),_style.separatorHeight);
        if( (chartRect().contains(event->pos()))||(volumeRect().contains(event->pos()))||(indicatorRect().contains(event->pos())) )
        {
            setCursor(Qt::ClosedHandCursor);
            _dragMode=dragTimeline;
            _gripTime=_cursorTime;
            _gripStartTime=_startTime;
            _gripEndTime=_endTime;
            _gripCursor=event->pos();
        }
        if(volumeUpRect.contains(event->pos()))
        {
            _dragMode   =dragVolumeUp;
            _gripCursor =event->pos();
            _gripHeight =volumeRect().height();
        }
        if(volumeDownRect.contains(event->pos()))
        {
            _dragMode   =dragVolumeDown;
            _gripCursor =event->pos();
            _gripHeight =indicatorRect().height();
        }
    }

}

//
void FinChartWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if(event->button()==Qt::LeftButton)
    {
        setCursor(Qt::CrossCursor);
        _dragMode=dragNone;
    }
}

//
void FinChartWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    QList<FinChartData*>::iterator i;
    for (i = _datas.begin(); i != _datas.end(); ++i)
    {
        //(*i)->mouseMoveEvent(event);
    }
    //if(event->pos())
    switch(_dragMode)
    {
        //
        case dragNone:
        {
            qint64      st=startTime().toSecsSinceEpoch();
            qint64      et=endTime().toSecsSinceEpoch();
            double      span=(double)(et-st);
            double      cpos=(double)event->pos().x()-chartRect().left();
            double      tpos=(cpos/((double)chartRect().width()))*span;
            qint64      t=st+((qint64)tpos);
            QDateTime   ctime=QDateTime::fromSecsSinceEpoch(t);
            if( (chartRect().contains(event->pos()))||(volumeRect().contains(event->pos()))||(indicatorRect().contains(event->pos())) )
            {
                // calc time
                //event->pos().x()-chartRect().left()
                _cursorTime=ctime;
                _cursorX=event->pos().x();
                _cursorY=event->pos().y();
                setCursor(Qt::CrossCursor);
            }
            else
            {
                _cursorY=-1;
                _cursorX=-1;
                QRect volumeUpRect(chartRect().left(),chartRect().bottom(),chartRect().width(),_style.separatorHeight);
                QRect volumeDownRect(chartRect().left(),volumeRect().bottom(),chartRect().width(),_style.separatorHeight);
                if( (volumeUpRect.contains(event->pos()))||(volumeDownRect.contains(event->pos())) )
                {
                    setCursor(Qt::SplitVCursor);
                }
                else
                {
                    setCursor(Qt::ArrowCursor);
                    //setCursor(Qt::SplitVCursor);
                }
            }
        }
        break;
        //
        case dragTimeline:
        {
            qint64      st=_gripStartTime.toSecsSinceEpoch();
            qint64      et=_gripEndTime.toSecsSinceEpoch();
            double      span=(double)(et-st);
            double      cpos=(double)event->pos().x()-chartRect().left();
            double      tpos=(cpos/((double)chartRect().width()))*span;
            qint64      t=st+((qint64)tpos);
            QDateTime   ctime=QDateTime::fromSecsSinceEpoch(t);
            // calc time diff
            qint64      grip=_gripTime.toSecsSinceEpoch();
            qint64      cursor=ctime.toSecsSinceEpoch();
            int         step=(_datas.length()&&_datas[0]->interval())?_datas[0]->interval():60;
            qint64      move=(grip-cursor);
            move-=move%step;

            //EG_TRACE(TAG,QString("%1 minutes diff").arg((cursor-grip)/60));
            _startTime=QDateTime::fromSecsSinceEpoch(_gripStartTime.toSecsSinceEpoch()+move);
            _endTime=QDateTime::fromSecsSinceEpoch(_gripEndTime.toSecsSinceEpoch()+move);
            emit timeRangeChanged(_startTime,_endTime,true);
        }
        break;
        //
        case dragVolumeUp:
        {
            int delta           =_gripCursor.y()-event->pos().y();
            int availableHeight =(int)(height()-(_style.marginBottom+_style.marginTop+_style.separatorHeight+_style.indicatorHeight));
            int minHeight       =qRound(availableHeight*0.1);
            int maxHeight       =qRound(availableHeight*0.5);
            _style.volumeHeight=qBound(minHeight,_gripHeight+delta,maxHeight);
            updateLayouts(NULL);
        }
        break;
        //
        case dragVolumeDown:
        {
            int delta           =_gripCursor.y()-event->pos().y();
            int availableHeight =(int)(height()-(_style.marginBottom+_style.marginTop+_style.separatorHeight+_style.volumeHeight));
            int minHeight       =qRound(availableHeight*0.1);
            int maxHeight       =qRound(availableHeight*0.5);
            _style.indicatorHeight=qBound(minHeight,_gripHeight+delta,maxHeight);
            updateLayouts(NULL);
        }
        break;
        //
    }


    invalidate();
}

//
void FinChartWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    /*if(!isVisible())
        return;*/
    QSize newSize=event->size();
    updateLayouts(&newSize);
    // need time recalc
    updateTimeRange(false);
    invalidate();
}

//
void FinChartWidget::showEvent(QShowEvent *event)
{
    //qDebug() << TAG << " " << "ShowEvent";
    //updateChart(true);
    //invalidate();
    //update();
    QWidget::showEvent(event);
}

//
void FinChartWidget::onStep()
{
    if(_needRedraw)
    {
        update();
        _needRedraw=false;
    }


}


