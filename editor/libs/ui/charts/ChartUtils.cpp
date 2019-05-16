#include "ChartUtils.h"
#include <QPalette>
#include <QStyle>

//
ChartUtils::ChartUtils(QWidget* widget,QPainter& painter) : QObject(widget), _painter(painter), _widget(widget)
{

}

//
void ChartUtils::paintSticker(const QRect& chartRect,QPoint p,QColor color,Qt::Orientation orientation,bool flip,QString text)
{
    // QPalette    palette(widget.style()->standardPalette());
    //const ::Style&  style=Application::instance().style();
    QPalette        palette(_widget->style()->standardPalette());
    int             textMargin=4;
    QFontMetrics    fm(_painter.font(), _painter.device());
    QRect           boundingRect    =fm.boundingRect(text);
    int             textHeight      =boundingRect.height();
    int             halfTextHeight  =(textHeight/2)+2;


    int nPoint=0;
    QPoint sticker[16];
    QPoint textPos;
    _painter.save();
    _painter.setPen(QColor(color.red(),color.green(),color.blue(),128));
    _painter.setBrush(palette.color(QPalette::Base));
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
        if(sticker[4].x()<=chartRect.left())
        {
            decalX=qMin(chartRect.left()-sticker[4].x(),maxDecal);
        }
        if(sticker[3].x()>=chartRect.right())
        {
            decalX=-qMin(sticker[3].x()-chartRect.right(),maxDecal);
        }

        sticker[2].rx()+=decalX;
        sticker[3].rx()+=decalX;
        sticker[4].rx()+=decalX;
        sticker[5].rx()+=decalX;

        // flip points (Y)
        if(flip)
        {
            for(int i=0;i<8;i++)
            {
                QPoint rel=sticker[i]-p;
                sticker[i]=p+QPoint(rel.x(),-rel.y());
            }
            textPos=QPoint(sticker[4].x()+textMargin,sticker[5].y()-(textMargin+1) );
        }
        else
        {
            textPos=QPoint(sticker[4].x()+textMargin,p.y()+(textHeight+2) );
        }

        nPoint=8;
    }
    _painter.drawConvexPolygon(sticker, nPoint);
    _painter.setPen(palette.color(QPalette::Text));
    _painter.drawText(textPos,text);
    _painter.restore();
}

