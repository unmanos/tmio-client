#include "StringsProvider.h"
EG_CORE_BEGIN(loc)

StringsProvider::StringsProvider()
{

}

//
void StringsProvider::setLanguage(Language l)
{
    _strings.clear();
    switch(l)
    {
        case Language::NotSet:
        break;
        case Language::English:
        applyEnglish();
        break;
        case Language::French:
        applyFrench();
        break;
    }
}

//
QString StringsProvider::getString(const QString& k)
{
    if(_strings.contains(k))
    {
        return(_strings[k]);
    }
    return(k);
}

//
void StringsProvider::applyEnglish()
{
    _strings["ApplicationStarted"]="Application started";
}

//
void StringsProvider::applyFrench()
{
    _strings["ApplicationStarted"]="L'application a démarrée";
}


EG_CORE_END()
