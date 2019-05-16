#ifndef CHANNEL_H
#define CHANNEL_H
#include "cm.h"
TM_CORE_BEGIN(cm)

//
class Subscriber;

//
class Channel : public QObject
{
    Q_OBJECT
public:
    explicit Channel(Subscriber* subscriber,const QString& name);

    QString         name();

signals:

public slots:

private:
    QString         _name;
};

TM_CORE_END()

#endif // CHANNEL_H
