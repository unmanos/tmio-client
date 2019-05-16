#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QWidget>
#include <QStyle>
#include "Style.h"

//
Style::Style(QObject *parent) : QObject(parent)
{
}

//
QPalette        Style::set(eStyle style)
{
    switch(style)
    {
        //
        case styleLight:
        return(set_Light());
        //
        case styleDark:
        return(set_Dark());
        //
    }
    return(QPalette());
}

//
QPalette        Style::palette() const
{
    return(qApp->palette());
}

//
QColor          Style::color(eColor color) const
{
    //return(QColor(255,128,0));
    return(_colors[color]);
}

//
QPalette        Style::set_Light()
{
    //QWidget     widget;
    qApp->setStyle(QStyleFactory::create("plastic"));
    QWidget     widget;
    QPalette    palette(widget.style()->standardPalette());
    qApp->setPalette(palette);
    // custom app colors
    _colors[colorTextChangeIncrease]    =QColor(0,128,0);
    _colors[colorTextChangeDecrease]    =QColor(128,0,0);
    _colors[colorCandleIncrease]        =QColor(64,196,64);
    _colors[colorCandleDecrease]        =QColor(196,64,64);
    _colors[colorTextTrace]             =QColor("darkgreen");
    _colors[colorTextInfo]              =QColor("darkblue");
    _colors[colorTextWarning]           =QColor("#B18904");
    _colors[colorTextError]             =QColor("darkred");

    return(palette);
}

//
QPalette        Style::set_Dark()
{
    // https://forum.qt.io/topic/64587/how-to-set-qcombobox-background-color-with-qpalette/4
    qApp->setStyle(QStyleFactory::create("plastic"));
    QPalette palette;

    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::ToolTipBase, QColor(53,53,53));
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);

    palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    palette.setColor(QPalette::HighlightedText, Qt::black);

    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);

    qApp->setPalette(palette);
    // custom app colors
    _colors[colorTextChangeIncrease]    =QColor(128,255,128);
    _colors[colorTextChangeDecrease]    =QColor(255,128,128);
    _colors[colorCandleIncrease]        =QColor(48,160,48);
    _colors[colorCandleDecrease]        =QColor(160,48,48);
    _colors[colorTextTrace]             =QColor("lightgreen");
    _colors[colorTextInfo]              =QColor("lightblue");
    _colors[colorTextWarning]           =QColor("yellow");
    _colors[colorTextError]             =QColor("LightCoral");

    return(palette);
}
