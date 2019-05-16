#include "Account.h"
#include "../rest/Connection.h"
#include "../rest/Request.h"

#define TAG "api::Account"

TM_CORE_BEGIN(api)

//
Account::Account(tmio::rest::Connection* connection,QJsonObject json) : QObject(connection)
{
    _connection     =connection;
    _json           =json;
    _id             =_json["_id"].toString();
    Q_ASSERT(QByteArray::fromHex(_id.toUtf8()).size()==16);
    _key            =_json["key"].toString();
    Q_ASSERT(QByteArray::fromHex(_key.toUtf8()).size()==16);
    _displayName    =_json["displayName"].toString();
    _username       =_json["username"].toString();
    if(_json.contains("Contact"))
    {
        _contact=new Contact(_json["Contact"].toObject());
    }
    QJsonArray      groups=_json["groups"].toArray();
    foreach(QJsonValue g,groups)
    {
        _groups.append(g.toString());
    }
}

//
Account::~Account()
{
    delete(_contact);
}

//
Contact*                Account::contact()
{
    return(_contact);
}

//
QString                 Account::id()
{
    return(_id);
}

//
QString                 Account::key()
{
    return(_key);
}

//
void                    Account::loadAvatar()
{
    _avatarImage.clear();
    if(_json.contains("avatarId"))
    {
        QString     avatarID=_json["avatarId"].toString();
        QString     datQuery="accounts/current/avatar";//?q=" + avatarID;
        TM_TRACE(TAG,QString("Loading avatar %1").arg(avatarID));
        rest::RequestResult result=_connection->rGET(datQuery);
        if(result._statusCode==200)
        {
            _avatarImage=result._raw;
        }
    }
}

//
void                    Account::loadProperties()
{
    _properties.clear();
    QString     datQuery="accounts/current/properties";
    TM_TRACE(TAG,QString("Loading properties"));
    rest::RequestResult result=_connection->rGET(datQuery);
    if(result._statusCode==200)
    {
        //_avatarImage=result._raw;
        QJsonArray      properties=result._document.array();
        foreach(QJsonValue p,properties)
        {
            QJsonObject     po      =p.toObject();
            const QString   key     =po["key"].toString();
            QVariant        value   =po["value"].toVariant();
            _properties.insert(key,value);
        }
    }
}

//
QVariant                Account::accountProperty(const QString& key)
{
    if(_properties.contains(key))
    {
        return(_properties[key]);
    }
    return(false);
}

//
void                    Account::setAccountProperty(const QString& key,bool value)
{
    rest::Request*      request =_connection->createRequest("accounts/current/property");
    QJsonObject         postData;
    postData.insert("key",key);
    postData.insert("value",value);
    connect(request,&rest::Request::finished,this,&Account::onSetPropertyResult);
    request->post(postData,false);
    _properties[key]=value;
    //request->
}

//
QString                 Account::username()
{
    return(_username);
}

//
bool                    Account::changeUsername(QString newUsername)
{
    QJsonObject             contents;
    contents.insert("name",newUsername);
    rest::RequestResult     result=_connection->rPOST("accounts/current/username",contents);
    if(result._statusCode==200)
    {
        _username=newUsername;
        emit changed();
        return(true);
    }
    return(false);
}

//
QString                 Account::displayName()
{
    return(_displayName);
}

//
bool                    Account::changeDisplayName(QString newDisplayName)
{
    QJsonObject             contents;
    contents.insert("name",newDisplayName);
    rest::RequestResult     result=_connection->rPOST("accounts/current/displayname",contents);
    if(result._statusCode==200)
    {
        _displayName=newDisplayName;
        emit changed();
        return(true);
    }
    return(false);
}

//
QByteArray              Account::avatarImage()
{
    return(_avatarImage);
}

//
bool                    Account::changeAvatarImage(QByteArray newAvatarData)
{
    rest::RequestResult result=_connection->rPOST("accounts/current/avatar",newAvatarData);
    if(result._statusCode==200)
    {
        _avatarImage=newAvatarData;
        emit changed();
        return(true);
    }
    return(false);
}

//
bool                    Account::isMemberOf(const QString& group)
{
    return(_groups.contains(group));
}

//
bool                    Account::removeGroup(const QString& group)
{
    return(_groups.removeAll(group)!=0);
}

//
QStringList             Account::groups()
{
    return(_groups);
}

//
QString                 Account::donateAddress(const QString& assetID)
{
    QJsonObject         das=_json["donateAddresses"].toObject();
    if(das.contains(assetID))
    {
        return(das[assetID].toString());
    }
    return(QString());
}

//
void                    Account::onSetPropertyResult(rest::RequestResult& result)
{
    result._request->deleteLater();
}

TM_CORE_END()
