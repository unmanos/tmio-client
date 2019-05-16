#include "Core.h"
#include <QDebug>
namespace tmio {

Core* Core::_instance=new Core();

//
Core&                   Core::instance()
{
    return(*_instance);
}

Core::Core()
{
}

Core::~Core()
{

}

//
void                    Core::info(QString tag,QString text)
{
    log(tag,logInfo,text);
}

//
void                    Core::trace(QString tag,QString text)
{
    log(tag,logTrace,text);
}

//
void                    Core::warning(QString tag,QString text)
{
    log(tag,logWarning,text);
}

//
void                    Core::error(QString tag,QString text)
{
    log(tag,logError,text);
}

//
void                    Core::log(QString tag,eLogType type,QString text)
{
    QString str="";
    if(tag.length())
    {
        str+=tag+" > ";
    }
    str+=text;
    emit sigLog(type,str);
    qDebug() << str.toStdString().c_str();
}

}
