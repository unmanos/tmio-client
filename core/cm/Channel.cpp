#include "Channel.h"
#include "Subscriber.h"
TM_CORE_BEGIN(cm)

//
Channel::Channel(Subscriber* subscriber,const QString& name) : QObject(subscriber), _name(name)
{

}

//
QString         Channel::name()
{
    return(_name);
}

TM_CORE_END()
