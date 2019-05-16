#include "IconnedDockStyle.h"

IconnedDockStyle::IconnedDockStyle(const QIcon& icon,  QStyle* style)
    : QProxyStyle(style)
    , icon_(icon)
{

}

IconnedDockStyle::~IconnedDockStyle()
{

}

void IconnedDockStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if(element == QStyle::CE_DockWidgetTitle)
    {
        //width of the icon
        int width = pixelMetric(QStyle::PM_ToolBarIconSize)*0.75;
        //margin of title from frame
        int margin = baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);

        QPoint icon_point(margin + option->rect.left(), margin + option->rect.center().y() - width/2);

        painter->drawPixmap(icon_point, icon_.pixmap(width, width));

        const_cast<QStyleOption*>(option)->rect = option->rect.adjusted(width, 0, 0, 0);
    }
    baseStyle()->drawControl(element, option, painter, widget);
}

