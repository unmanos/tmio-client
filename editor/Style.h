#ifndef STYLE_H
#define STYLE_H
#include <QObject>
#include <QColor>
#include <QPalette>

//
class Style : public QObject
{
    Q_OBJECT
public:

    enum eStyle
    {
        styleLight,
        styleDark
    };

    enum eColor
    {
        colorTextChangeIncrease,
        colorTextChangeDecrease,
        colorCandleIncrease,
        colorCandleDecrease,
        colorTextTrace,
        colorTextInfo,
        colorTextWarning,
        colorTextError,
        color_MAX
    };

    Style(QObject *parent = nullptr);

    QPalette        set(eStyle);
    QPalette        palette() const;
    QColor          color(eColor) const;

private:
    QPalette        set_Light();
    QPalette        set_Dark();

signals:

public slots:

private:
    QColor          _colors[color_MAX];
};

#endif // STYLE_H
