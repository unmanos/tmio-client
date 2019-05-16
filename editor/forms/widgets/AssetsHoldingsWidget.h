#ifndef ASSETSHOLDINGSWIDGET_H
#define ASSETSHOLDINGSWIDGET_H
#include <QLabel>
#include <QWidget>
#include <QStandardItemModel>

//
class AssetsHoldingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AssetsHoldingsWidget(QWidget *parent = nullptr);

private:
    void                            refreshHoldings();

signals:

public slots:
    void                            onUpdate();
    void                            onWalletsChanged();
    void                            onCurrentRowChanged(QModelIndex index,QModelIndex index2);

private:
    typedef QList<QStandardItem*>   Row;
    QStandardItemModel*             _model;
    bool                            _upToDate               =false;
    QLabel*                         _labelTotalBalance      =nullptr;
    QLabel*                         _labelTotalBalanceQuote =nullptr;
    QMap<QString,Row>               _rows;
};

#endif // ASSETSHOLDINGSWIDGET_H
