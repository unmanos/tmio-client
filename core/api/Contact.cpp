#include "Contact.h"
TM_CORE_BEGIN(api)

Contact::Contact(QJsonObject json)
{
    _json=json;
}

//
Contact::~Contact()
{

}

//
QString Contact::name()
{
    QString str="";
    if(_json.contains("Person"))
    {
        QJsonObject person=_json["Person"].toObject();
        str=person["FirstName"].toString();
        str+=" "+person["LastName"].toString();
    }
    if(_json.contains("Organization"))
    {
        QJsonObject org=_json["Organization"].toObject();
        str=org["Name"].toString();
        //str+=" "+person["LastName"].toString();
    }
    return(str);
}

//
Contact::eType Contact::type()
{
    if(_json.contains("Organization"))
    {
        QJsonObject organization=_json["Organization"].toObject();
        QString orgType=organization["Type"].toString();
        if(orgType=="Public")
            return(tPublic);
        return(tBusiness);
    }
    if(_json.contains("Person"))
    {
        return(tPerson);
    }
    return(tUnknown);
}

TM_CORE_END()
