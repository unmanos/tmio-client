#ifndef MARKETDATAEDITASSETDIALOG_H
#define MARKETDATAEDITASSETDIALOG_H
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QTreeView>
#include <rest/Connection.h>
#include <md/Source.h>

using namespace tmio;

namespace Ui {
class MarketDataEditAssetDialog;
}

//
class MarketDataEditAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MarketDataEditAssetDialog(const QString& assetID,QWidget *parent);
    ~MarketDataEditAssetDialog();

    virtual void                    accept();

    struct                  Source
    {
        QString                     name;
        QComboBox*                  combo;
        QStandardItemModel*         comboModel;
        QTreeView*                  comboTree;
        QLineEdit*                  filter;
        QLineEdit*                  circulatingSupply;
        QLineEdit*                  priceBtc;
        QLineEdit*                  identifier;
        QLineEdit*                  symbol;
        QString                     selectedCoinID;
    };

private:
    void                            loadAsset();
    void                            refreshCombo(Source& source);
    void                            refreshSourceData(Source& source);

public slots:
    void                            onSourceFilterChanged(const QString& text);
    void                            onSourceCoinChanged(int);
    void                            onAssetTypeChanged(int);
    void                            onAssetExchangeChanged(int);

private:
    Ui::MarketDataEditAssetDialog * ui;
    QString                         _assetID;
    QMap<QString,Source>            _sources;
    QMap<QString,QString>           _exchanges;
    bool                            _createAsset;
};

#endif // MARKETDATAEDITASSETDIALOG_H
