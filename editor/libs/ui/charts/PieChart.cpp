#include "PieChart.h"
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QStyle>

//
PieChart::PieChart(QWidget *parent) : QWidget(parent)
{
    _defaultColors.append(QColor("#C9050B"));
    _defaultColors.append(QColor("#F67600"));
    _defaultColors.append(QColor("#F5BA01"));
    _defaultColors.append(QColor("#6EB325"));
    _defaultColors.append(QColor("#2F7DDF"));
    _defaultColors.append(QColor("#6018C8"));

    setMouseTracking(true);

    _updateTimer        =new QTimer();
    _updateTimer->setInterval(1000/60);
    connect(_updateTimer,&QTimer::timeout,this,&PieChart::onUpdate);

    _initAngle      =M_PI+M_PI/2.0;
}

//
PieChart::~PieChart()
{

}

//
void                    PieChart::resetValues()
{
    _total          =0;
    _upToDate       =false;
    _values.clear();
}

//
void                    PieChart::setValue(const QString& id,const QString& name,double amount)
{
    setValue(id,name,QIcon(),amount);
}

//
void                    PieChart::setValue(const QString& id,const QString& name,QIcon icon,double amount)
{
    bool            found=false;
    for(int i=0;i<_values.size();i++)
    {
        if(_values[i].id==id)
        {
            _total          -=_values[i].amount;
            _values[i].amount   =amount;
            _values[i].icon     =icon;
            found=true;
            break;
        }
    }
    if(!found)
    {
        Value           v;
        v.id            =id;
        v.name          =name;
        v.icon          =icon;
        v.amount        =amount;
        v.color         =_defaultColors.at(_values.size()%_defaultColors.size());
        _values.append(v);
    }
    _total          +=amount;
    // sort
    std::sort(_values.begin(), _values.end(),[](const Value& a, const Value& b) -> bool { return a.amount < b.amount; });
    //
    _upToDate           =false;
    update();
}

//
double                  PieChart::posToAngle(const QPoint& p,double* plength)
{
    int             size=qMin(width(),height())-_margin*2;
    QPointF         pieCenter( (_margin*2+size)/2,(_margin*2+size)/2);
    double          px      =static_cast<double>(p.x()-pieCenter.x())/static_cast<double>(size/2);
    double          py      =static_cast<double>(p.y()-pieCenter.y())/static_cast<double>(size/2);
    double          length  =qSqrt(px*px+py*py);
    double          angle   =qAtan2(py/length,px/length);
    if(plength)
        *plength=length;
    return(angle);
}

//
void                    PieChart::updateDrawValues()
{
    double  angle       =_initAngle;
    for(int i=0;i<_values.size();i++)
    {
        Value&          v=_values[i];
        double          span=(v.amount/_total)*M_PI*2;
        v.dist          =(v.amount/_total)*M_PI;
        v.angle         =angle-span/2;
        angle           -=span;
    }
}

//
/*static double           _angleToRad(double angle)
{
    return (M_PI-( (angle/(1440.0*2))*M_PI))+M_PI;
}*/

//
static double           _radNorm(double a)
{
    return(qAtan2(qSin(a),qCos(a)));
}

//
static double           _radDist(double a, double b)
{
    return(qAbs(_radNorm(a-b)));
}


//
void                    PieChart::paintEvent(QPaintEvent* event)
{
    Q_ASSERT(_values.size());
    if(!_upToDate)
    {
        updateDrawValues();
        _upToDate=true;
    }

    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    QPointF         center(width()/2,height()/2);
    int             size=qMin(width(),height())-_margin*2;
    QPointF         pieCenter( (_margin*2+size)/2,(_margin*2+size)/2);
    for(int ipass=0;ipass<2;ipass++)
    {
        foreach(const Value& v,_values)
        {
            const bool      active  =(v.id==_activeValue)?true:false;
            qDebug()<< v.id;
            if( (ipass==0)&&(active) )
                continue;
            if( (ipass==1)&&(!active) )
                continue;

            QColor          color   =active?v.color.lighter(110):v.color;
            const double    outerRadius  =(static_cast<double>(size)/2.0)*(active?1.02:1.0);
            const double    innerRadius  =(static_cast<double>(size)/2.0)*_innerRadius;
            painter.setPen(color);
            painter.setBrush(QBrush(color));
            const double  cx=qCos(v.angle);
            const double  cy=qSin(v.angle);
            QPainterPath            donut;
            double outerDist        =v.dist-0.01;
            double innerDist        =v.dist-(0.01*(1/_innerRadius));
            donut.moveTo(qCos(v.angle-outerDist)*outerRadius,qSin(v.angle-outerDist)*outerRadius);
            const int               nStep=static_cast<int>(qBound(8.0,(size/3)/((M_PI)/v.dist),256.0));
            for(int i=0;i<nStep;i++)
            {
                double  t=static_cast<double>(i+1)/static_cast<double>(nStep);
                double  ta=(v.angle-outerDist)+t*outerDist*2;
                donut.lineTo(qCos(ta)*outerRadius,qSin(ta)*outerRadius);
            }
            for(int i=0;i<=nStep;i++)
            {
                double  t=(static_cast<double>(i)/static_cast<double>(nStep));
                double  ta=(v.angle+innerDist)-(t*innerDist*2);
                donut.lineTo(qCos(ta)*innerRadius,qSin(ta)*innerRadius);
            }
            //donut.lineTo(qCos(v.angle-v.dist)*outerRadius,qSin(v.angle-v.dist)*outerRadius);
            donut.translate(pieCenter);

            if(active)
            {
                //painter.fillPath(donut,QBrush(QColor(128,0,0)));
                //painter.setPen(QColor(255,255,255));
                //painter.drawPath(donut);
                /*painter.setPen(QColor(255,255,255));
                painter.setBrush(Qt::BrushStyle::NoBrush);
                painter.drawPie(_margin,_margin,size,size,angle,span+0.5);*/
            }
            painter.fillPath(donut,QBrush(color));
            QPointF textPos=pieCenter+QPointF(cx*(outerRadius+20),cy*(outerRadius+20));

            QFontMetrics    fm(painter.font(), painter.device());
            QRect           textRect    =fm.boundingRect(v.name);


            double moveX=static_cast<double>(textRect.width())*cx*0.5;
            double moveY=static_cast<double>(textRect.height())*cy*0.5;
            textRect.moveTo(static_cast<int>(textPos.x()+moveX-textRect.width()/2),
                            static_cast<int>(textPos.y()+moveY-textRect.height()/2));
            //textRect.move

            //style()->
            painter.setPen(QColor(0,0,0));
            painter.setBrush(QBrush(QWidget::palette().color(QWidget::backgroundRole())));
            painter.drawLine(pieCenter+QPointF(cx*outerRadius,cy*outerRadius),textPos);
            painter.drawRect(textRect.adjusted(-3,-2,3,2));
            //painter.drawText(textPos,dv.name);
            painter.drawText(textRect.adjusted(0,0,1,1),v.name);

            const double  imageX=pieCenter.x()+cx*(outerRadius+innerRadius)*0.5;
            const double  imageY=pieCenter.y()+cy*(outerRadius+innerRadius)*0.5;
            //painter.drawImage(v.icon.pixmap());
            painter.drawPixmap(QRect(static_cast<int>(imageX),static_cast<int>(imageY),0,0).adjusted(-10,-10,10,10),v.icon.pixmap(20,20));

            if(active)
            {
                QString         detailsText         =QString("%1\r\n%2%").arg(v.name).arg(QString::number((v.amount/_total)*100.0,'f',1));
                const int       iri                 =static_cast<int>(innerRadius);
                QRect           detailsTextRect     =QRect(pieCenter.toPoint(),QSize(0,0)).adjusted(-iri,-iri,iri,iri);
                QFont           detailsFont=font();
                detailsFont.setBold(true);
                detailsFont.setPointSize(static_cast<int>(detailsFont.pointSizeF()*1.3));
                //setFont(detilsFont);
                painter.setFont(detailsFont);
                painter.drawText(detailsTextRect,Qt::AlignCenter,detailsText);
            }

            //painter.resetTransform();

            //angle           +=span;
        }
    }
}

//
void                    PieChart::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

//
void                    PieChart::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if(event->button()==Qt::LeftButton)
    {
        double          length;
        _gripAngle      =posToAngle(event->pos(),&length);
        if( (length>=_innerRadius)&&(length<=1) )
        {
            //_activeValue.clear();
            _grip           =true;
            _gripInitAngle  =_initAngle;
        }
    }
}

//
void                    PieChart::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if(event->button()==Qt::LeftButton)
    {
        _grip           =false;
        mouseMoveEvent(event);
    }
}

//
void                    PieChart::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    double          length  =0;
    double          angle   =posToAngle(event->pos(),&length);
    if(!_grip)
    {
        QString         activeDv;
        if( (length>=_innerRadius)&&(length<=1) )
        {
            foreach(Value v,_values)
            {
                if(_radDist(angle,v.angle)<v.dist)
                {
                    activeDv=v.id;
                }
                else
                {

                }
            }
        }
        if(_activeValue!=activeDv)
        {
            _activeValue=activeDv;
            qDebug() << _activeValue;
            //_updateTimer->start();
            update();
        }
    }
    else
    {
        if(length>=0.001)
        {
            _initAngle      =_gripInitAngle+(angle-_gripAngle);
            _upToDate       =false;
            update();
        }
    }
}

//
void                    PieChart::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

//
void                    PieChart::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

//
void                    PieChart::onUpdate()
{
}
