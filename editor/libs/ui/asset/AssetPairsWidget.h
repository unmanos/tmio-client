#ifndef ASSETPAIRSWIDGET_H
#define ASSETPAIRSWIDGET_H
#include <QWidget>
#include <QStandardItemModel>

//
class AssetPairsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AssetPairsWidget(const QString& assetID,QWidget *parent = nullptr);

private:
    void                            refreshPairs();

signals:
    void                            openPair(const QString& exchange,const QString& pair);

public slots:
    void                            onPriceChanged();
    void                            onQuoteAssetChanged();
    void                            onFavoriteTickerChanged(const QString& exchange,const QString& ticker,bool favorite);
    void                            onPairItemChanged(QStandardItem*);
    void                            onOpenPair(QModelIndex);

private:
    QString                         _assetID;
    QStandardItemModel*             _pairsModel         =nullptr;
    QMap<QString,QStandardItem*>    _pairsLookup;
};

#endif // ASSETPAIRSWIDGET_H
