#ifndef ASSETVIEW_H
#define ASSETVIEW_H
#include <QWidget>
#include <QLabel>
#include <QStandardItemModel>

//
class AssetView : public QWidget
{
    Q_OBJECT
public:
    explicit AssetView(QString assetID,QWidget *parent=nullptr);

signals:

public slots:
    void                onPriceChanged();

private:
    QString             _assetID;
    QLabel*             _labelLastPrice     =nullptr;
    QLabel*             _labelMarketCap     =nullptr;
};

#endif // ASSETVIEW_H
