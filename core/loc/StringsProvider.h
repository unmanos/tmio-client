#ifndef STRINGSPROVIDER_H
#define STRINGSPROVIDER_H
#include "loc.h"
EG_CORE_BEGIN(loc)

class StringsProvider
{
public:
    StringsProvider();
    void setLanguage(Language);
    QString getString(const QString& k);
private:
    void applyEnglish();
    void applyFrench();

    QMap<QString, QString> _strings;
};

EG_CORE_END()
#endif // STRINGSPROVIDER_H
