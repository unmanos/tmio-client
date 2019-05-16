#ifndef ICONNEDDOCKSTYLE_H
#define ICONNEDDOCKSTYLE_H
#include <QProxyStyle>
#include <QPainter>
#include <QStyleOption>

// https://stackoverflow.com/questions/2386527/how-to-display-icon-in-qdockwidget-title-bar
class IconnedDockStyle: public QProxyStyle {
    Q_OBJECT
    QIcon icon_;
public:
    IconnedDockStyle(const QIcon& icon,  QStyle* style = 0);

    virtual ~IconnedDockStyle();

    virtual void drawControl(ControlElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget = 0) const;
};



#endif // ICONNEDDOCKSTYLE_H
