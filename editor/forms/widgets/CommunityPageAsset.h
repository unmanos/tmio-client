#ifndef COMMUNITYPAGEASSET_H
#define COMMUNITYPAGEASSET_H

#include <QWidget>

class CommunityPageAsset : public QWidget
{
    Q_OBJECT
public:
    explicit CommunityPageAsset(const QString& assetID,QWidget *parent = nullptr);
    virtual ~CommunityPageAsset();


signals:

public slots:

private:
    QString         _channel;
};

#endif // COMMUNITYPAGEASSET_H
