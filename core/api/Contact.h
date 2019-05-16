#pragma once
#include "../Core.h"
#include <QObject>
#include <QJsonObject>

TM_CORE_BEGIN(api)

class Contact : public QObject
{
    Q_OBJECT
public:
    Contact(QJsonObject json);
    virtual ~Contact();

    enum eType
    {
        tUnknown,
        tPerson,
        tPublic,
        tBusiness,
    };

    QString             name();
    eType               type();

    public slots:

    signals:

    private:
    QJsonObject         _json;
};

TM_CORE_END()
