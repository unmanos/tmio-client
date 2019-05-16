#include "LineChart.h"
#include "ChartUtils.h"
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <QDateTime>
#include <math.h>

//
static double           _sqlinedist(const QPointF& a,const QPointF& b,const QPointF& p,double& t)
{
    const QPointF   d	=	b-a;
    const double    num	=	QPointF::dotProduct(d,d);
    if(qAbs(num)>0.000000000000000000001)
    {
        const double    den	=	-QPointF::dotProduct(d,a-p);
        t=den/num;
    }
    if(t<0)
    {
        return(QPointF::dotProduct(p-a,p-a));
    }
    if(t>1)
    {
        return(QPointF::dotProduct(p-b,p-b));
    }
    const QPointF   ip  =(a+d*t)-p;
    return(QPointF::dotProduct(ip,ip));
}

//
LineChart::LineChart(QWidget *parent) : QWidget(parent)
{
    _defaultColors.append(QColor("#C9050B"));
    _defaultColors.append(QColor("#F67600"));
    _defaultColors.append(QColor("#F5BA01"));
    _defaultColors.append(QColor("#6EB325"));
    _defaultColors.append(QColor("#2F7DDF"));
    _defaultColors.append(QColor("#6018C8"));
    setMouseTracking(true);
}

//
void                    LineChart::addDataSet(const QString& name,eSide side)
{
    _dataSets[name].name        =name;
    _dataSets[name].side        =side;
    _dataSets[name].color       =_defaultColors[(_dataSets.size()-1)%_defaultColors.size()];
    _dataSetsUpdated            =false;
}

//
bool                    LineChart::setDataPoint(const QString& setName,QVariant x,QVariant y)
{
    Q_ASSERT(_dataSets.contains(setName));
    Q_ASSERT(x.type()==_horizontalUnitsType);
    Q_ASSERT(y.type()==_verticalUnitsType[_dataSets[setName].side]);
    if( (!_dataSets.contains(setName))||
        (x.type()!=_horizontalUnitsType)||
        (y.type()!=_verticalUnitsType[_dataSets[setName].side]) )
    {
        return(false);
    }
    _dataSets[setName].points[x]=y;
    _dataSetsUpdated            =false;
    return(true);
}

//
void                    LineChart::setHorizontalUnits(QVariant::Type type)
{
    Q_ASSERT(_dataSets.size()==0);
    _horizontalUnitsType            =type;
    _dataSets.clear();
}

//
void                    LineChart::setVerticalUnits(eSide side,QVariant::Type type)
{
    Q_ASSERT(_dataSets.size()==0);
    _verticalUnitsType[side]        =type;
    _dataSets.clear();
}

//
void                    LineChart::setFormatter(FormatFn fn)
{
    _formatFn                       =fn;
}

//
void                    LineChart::updateDataSets()
{
    if(!_dataSetsUpdated)
    {
        _xmin=_xmax         =QVariant();
        _ymin[0]=_ymin[1]   =QVariant();
        _ymax[0]=_ymax[1]   =QVariant();
        Q_ASSERT(_xmin.isNull());
        // update data sets ranges
        foreach(const QString& dataSetName,_dataSets.keys())
        {
            DataSet&        set=_dataSets[dataSetName];
            if(set.points.size())
            {
                set.xmin=set.points.firstKey();
                set.xmax=set.points.lastKey();
                set.ymin=set.ymax=set.points.first();
                foreach(QVariant k,set.points.keys())
                {
                    set.ymin        =qMin(set.ymin,set.points[k]);
                    set.ymax        =qMax(set.ymax,set.points[k]);
                }
                /*qDebug() << set.name;
                qDebug() << "\t" << set.xmin << "<= x <=" << set.xmax;
                qDebug() << "\t" << set.ymin << "<= y <=" << set.ymax;*/
            }
            // global x range
            _xmin               =_xmin.isNull()?set.xmin:qMin(_xmin,set.xmin);
            _xmax               =_xmax.isNull()?set.xmax:qMax(_xmax,set.xmax);
            // global y range
            _ymin[set.side]     =_ymin[set.side].isNull()?set.ymin:qMin(_ymin[set.side],set.ymin);
            _ymax[set.side]     =_ymax[set.side].isNull()?set.ymax:qMax(_ymax[set.side],set.ymax);
        }
        // update screen points
        foreach(const QString& dataSetName,_dataSets.keys())
        {
            DataSet&        set =_dataSets[dataSetName];
            set.spoints         =getDataSetPoints(set).toVector();
        }
        _dataSetsUpdated=true;
    }
}

//
QString                 LineChart::formatValue(eSide side,eAxe axe,QVariant value)
{
    QString     text;
    if(_formatFn)
    {
        text=_formatFn(side,axe,value);
    }
    if(text.length()==0)
    {
        //text=QString::number(gridCurrent,'f',decimals);
        text=value.toString();
    }
    return(text);
}

//
double                  LineChart::getWeight(const QVariant& min,const QVariant& value,const QVariant& max)
{
    Q_ASSERT(min.type()==value.type());
    Q_ASSERT(max.type()==value.type());
    switch(value.type())
    {
        //
        case QVariant::Type::Double:
        {
            const double        cmin=min.toDouble();
            const double        cmax=max.toDouble();
            const double        cval=value.toDouble();
            return( (cval-cmin)/(cmax-cmin) );
        }
        //
        case QVariant::Type::Int:
        {
            const double        cmin=static_cast<double>(min.toInt());
            const double        cmax=static_cast<double>(max.toDouble());
            const double        cval=static_cast<double>(value.toDouble());
            return( (cval-cmin)/(cmax-cmin) );
        }
        //
        case QVariant::Type::DateTime:
        {
            const double        cmin=static_cast<double>(min.toDateTime().toSecsSinceEpoch());
            const double        cmax=static_cast<double>(max.toDateTime().toSecsSinceEpoch());
            const double        cval=static_cast<double>(value.toDateTime().toSecsSinceEpoch());
            return( (cval-cmin)/(cmax-cmin) );
        }
        //
        default:
        Q_ASSERT(0);
    }
    return(0);
}

//
QList<QPointF>          LineChart::getDataSetPoints(const DataSet& dataset)
{
    QList<QPointF>      path;
    const double        ox  =static_cast<double>(_chartRect.left());
    const double        oy  =static_cast<double>(_chartRect.top());
    const double        w   =static_cast<double>(_chartRect.width());
    const double        h   =static_cast<double>(_chartRect.height());
    foreach(QVariant v,dataset.points.keys())
    {
        const double                  x=ox+getWeight(_xmin,v,_xmax)*w;
        const double                  y=oy+(1-getWeight(_ymin[dataset.side],dataset.points[v],_ymax[dataset.side]))*h;
        path.append(QPointF(x,y));
    }
    return(path);
}

//
void                    LineChart::updateRects()
{
    _dataSetsUpdated            =false;
    QRect                       oldRect=_chartRect;
    _chartRect=QRect(QPoint(0,0),size());
    _chartRect.adjust(_horizontalMargin,20,-_horizontalMargin,-20);
    _chartFrameRect=QRect(QPoint(0,0),size());
    _chartFrameRect.adjust(_horizontalMargin,10,-_horizontalMargin,-10);
    if(oldRect!=_chartRect)
    {
        update();
    }
}

//
void                    LineChart::paintDataSets(QPainter& painter)
{
    ChartUtils          chartUtils(this,painter);
    foreach(const QString& dataSetName,_dataSets.keys())
    {
        if(dataSetName!=_selectedSet)
        {
            DataSet&        set     =_dataSets[dataSetName];
            QPainterPath    path;
            //QList<QPointF>  points  =getDataSetPoints(set);
            if(set.spoints.size())
            {
                path.moveTo(set.spoints[0]);
                for(int p=1;p<set.points.size();p++)
                {
                    path.lineTo(set.spoints[p]);
                }
                //painter.setPen(set.color);
                painter.setPen(QPen(set.color, 1.5));
                //painter.setBrush(QBrush(set.color));
                painter.drawPath(path);
                //painter.drawLine(set.spoints[_selectedPoint]);

            }
        }
    }
    if(_dataSets.contains(_selectedSet))
    {
        DataSet&        set     =_dataSets[_selectedSet];
        //QList<QPointF>  points  =getDataSetPoints(set);
        if(set.spoints.size())
        {
            QPainterPath    path;
            painter.setPen(QPen(set.color, 2.5));
            path.moveTo(set.spoints[0]);
            for(int p=1;p<set.points.size();p++)
            {
                path.lineTo(set.spoints[p]);
            }
            painter.drawPath(path);
        }
        if(_selectedPoint>=0)
        {
            const QPointF       pt=set.spoints[_selectedPoint];
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(set.color));
            painter.drawEllipse(pt,4,4);
            painter.setBrush(Qt::NoBrush);
            bool                flip=false;
            if(_cursorPos.y()>pt.y())
                flip=true;
            if( (pt.y()-40)<_chartFrameRect.top())
                flip=false;
            if( (pt.y()+40)>_chartRect.bottom())
                flip=true;
            chartUtils.paintSticker(_chartRect,QPoint(pt.x(),pt.y()+(flip?-5:5)),QColor(0,0,0),Qt::Orientation::Vertical,flip,"Hello world!");
        }
    }
}

//
void                    LineChart::paintVerticalScales(QPainter& painter)
{
    painter.setPen(QColor(0,0,0));
    const double scaleTextMargin       =2;
    const double scaleMarkerSize       =6;
    for(int iScale=0;iScale<2;iScale++)
    {
        if( (_ymin[iScale].isNull())||(_ymax[iScale].isNull()) )
            continue;
        bool            unknownScale    =false;
        double          low             =0;
        double          high            =0;
        double          gridStep        =0;
        double          span            =0;
        switch(_ymin[iScale].type())
        {
            case QVariant::Type::Double:
            low             =_ymin[iScale].toDouble();
            high            =_ymax[iScale].toDouble();
            gridStep        =0.00000001;
            span            =high-low;
            break;
            case QVariant::Type::Int:
            low             =_ymin[iScale].toInt();
            high            =_ymax[iScale].toInt();
            gridStep        =1;
            span            =high-low;
            break;
            default:
            unknownScale        =true;
            break;
        }
        if(unknownScale)
            continue;
        int             perfectCount    =_chartRect.height()/40;
        double          nStep           =0;
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
        double      gridStart=low-fmod(low,gridStep);
        double      gridCurrent=gridStart;
        while(gridCurrent<high)
        {
            // get point
            const double      y=getWeight(low,gridCurrent,high);
            const double      px=iScale*_chartRect.width()+_chartRect.left();
            const double      py=(1-y)*_chartRect.height()+_chartRect.top();
            if( (y>0)&&(y<1) )
            {
                //const QString       text=QString::number(gridCurrent,'f',decimals);
                const QString       text=formatValue(static_cast<eSide>(iScale),eAxe::Vertical,gridCurrent);
                painter.drawLine(QPointF(px-scaleMarkerSize/2,py),QPointF(px+scaleMarkerSize/2,py));

                QFontMetrics    fm(painter.font(), painter.device());
                QRect           boundingRect    =fm.boundingRect(text);
                QPointF         textPos         =QPointF(px,py);
                if(iScale==0)
                {
                    textPos+=QPointF(-(boundingRect.width()+scaleMarkerSize/2+scaleTextMargin),boundingRect.height()/4.0);
                }
                if(iScale==1)
                {
                    textPos+=QPointF(scaleMarkerSize/2+scaleTextMargin,boundingRect.height()/4.0);
                }

                painter.drawText(textPos,text);
                _horizontalMargin       =qMax(_horizontalMargin,boundingRect.width()+10);
            }
            gridCurrent+=gridStep;
        }
    }
}

//
void                    LineChart::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    updateDataSets();
    // y //
    paintDataSets(painter);
    paintVerticalScales(painter);

    painter.setPen(QColor(0,0,0));
    painter.drawRect(_chartFrameRect);
    updateRects();
}

//
void                    LineChart::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

//
void                    LineChart::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
}

//
void                    LineChart::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
}

//
void                    LineChart::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    updateDataSets();
    double      bestDist        =500;
    QString     selectedSet;
    int         selectedPoint   =-1;
    foreach(const QString& dataSetName,_dataSets.keys())
    {
        const DataSet&  set     =_dataSets[dataSetName];
        //QPainterPath    path;
        //QList<QPointF>  points  =getDataSetPoints(set);
        if(set.spoints.size()>1)
        {
            for(int i=0;i<(set.spoints.size()-1);i++)
            {
                double       t;
                const double dist       =_sqlinedist(set.spoints[i],set.spoints[i+1],event->pos(),t);
                if(dist<bestDist)
                {
                    bestDist        =dist;
                    selectedSet     =dataSetName;
                    selectedPoint   =(t<0.5)?i:(i+1);
                }
            }
            //static double _sqlinedist(const QPointF& a,const QPointF& b,const QPointF& p)
        }
    }
    bool forceUpdate=false;
    if(selectedPoint>=0)
    {
        // force update if cursor changed down/upside
        bool    way0=(_dataSets[selectedSet].spoints[selectedPoint].ry()-event->pos().ry())<0;
        bool    way1=(_dataSets[selectedSet].spoints[selectedPoint].ry()-_cursorPos.ry())<0;
        if(way0!=way1)
            forceUpdate=true;
    }
    if( (_selectedSet!=selectedSet)||(_selectedPoint!=selectedPoint)||(forceUpdate) )
    {
        _selectedSet    =selectedSet;
        _selectedPoint  =selectedPoint;
        qDebug() << selectedSet;
        update();
    }
    _cursorPos=event->pos();
    //update();
}

//
void                    LineChart::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateRects();
}

//
void                    LineChart::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}
