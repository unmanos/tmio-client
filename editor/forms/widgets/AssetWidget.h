#ifndef ASSETWIDGET_H
#define ASSETWIDGET_H

#include <QWidget>

class AssetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AssetWidget(const QString& assetID,QWidget *parent = nullptr);

signals:

public slots:

private:
    QString             _assetID;
};

#endif // ASSETWIDGET_H
