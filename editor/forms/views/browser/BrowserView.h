#ifndef BROWSERVIEW_H
#define BROWSERVIEW_H

#include <QWidget>

class BrowserView : public QWidget
{
    Q_OBJECT
public:
    explicit BrowserView(const QString& url,const QString& title,QWidget *parent = nullptr);

signals:

public slots:
};

#endif // BROWSERVIEW_H
