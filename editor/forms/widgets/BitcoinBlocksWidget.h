#ifndef BITCOINBLOCKSWIDGET_H
#define BITCOINBLOCKSWIDGET_H
#include <QFrame>
#include <rest/Request.h>

//
class QTreeView;
class QStandardItemModel;
class QStandardItem;

//
class BitcoinBlocksWidget : public QFrame
{
public:
    BitcoinBlocksWidget();

protected:
    void                            updateBlock(QJsonObject block);

public slots:
    void                            onAssetModified(const QString& id);
    void                            onRequestBlocks();
    void                            onReceiveBlocks(tmio::rest::RequestResult&);

private:
    typedef QList<QStandardItem*>   Row;
    QMap<int,Row>                   _rows;
    QTreeView*                      _treeView       =nullptr;
    QStandardItemModel*             _model          =nullptr;
    tmio::rest::Request*            _blocksRequest  =nullptr;
};

#endif // BITCOINBLOCKSWIDGET_H
