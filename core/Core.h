#pragma once
#include <QObject>
#include <QMap>
#include <QException>
#include <QString>
#include <QVector>
#include <QJsonArray>

#define TM_CORE_BEGIN(__ns) namespace tmio { namespace __ns {
#define TM_CORE_END() }}

namespace tmio {

class Core : public QObject
{
    Q_OBJECT
public:
    enum eLogType
    {
        logInfo,
        logTrace,
        logWarning,
        logError
    };

    static Core&                instance();

    Core();
    virtual ~Core();

    void                        info(QString tag,QString text);
    void                        trace(QString tag,QString text);
    void                        warning(QString tag,QString text);
    void                        error(QString tag,QString text);

private:
    void                        log(QString tag,eLogType type,QString text);

signals:
    void                        sigLog(tmio::Core::eLogType,QString text);

private:
    static Core*                _instance;
};

}


#define TM_INFO(__tag,__text)       tmio::Core::instance().info(__tag,__text)
#define TM_TRACE(__tag,__text)      tmio::Core::instance().trace(__tag,__text)
#define TM_WARNING(__tag,__text)    tmio::Core::instance().warning(__tag,__text)
#define TM_ERROR(__tag,__text)      tmio::Core::instance().error(__tag,__text)
