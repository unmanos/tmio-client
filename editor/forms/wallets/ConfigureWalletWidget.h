#ifndef CONFIGUREWALLETWIDGET_H
#define CONFIGUREWALLETWIDGET_H
#include <QWidget>
#include <QJsonObject>
#include <QMap>

//
class QFormLayout;
class QComboBox;
class QLineEdit;

//
class ConfigureWalletWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigureWalletWidget(int walletID,QJsonObject walletConfig);

    QJsonObject             config();

    struct                  Argument
    {
        QString             path;
        QString             name;
        QString             text;
        QString             value;
    };

private:
    void                    setExchangeArgument(const QString& exchange,const QString& name,const QString& text);
    void                    setMiningPoolArgument(const QString& pool,const QString& name,const QString& text);
    void                    setPrivateArgument(const QString& asset,const QString& name,const QString& text);
    void                    setJsonRpcArgument(const QString& asset,const QString& name,const QString& text);
    void                    setArgumentValue(const QString& key,const QString& text);
    void                    updateChanges();
    void                    createArgsControls(const QString& startPath);

signals:
    void                    walletChanged(int walletID,QJsonObject walletConfig);

public slots:
    void                    onTypeChanged(int);
    void                    onNameChanged(const QString& text);
    void                    onExchangeChanged(int);
    void                    onMiningPoolChanged(int);
    void                    onPrivateCoinChanged(int);
    void                    onJsonRpcCoinChanged(int);
    void                    onArgumentChanged(const QString& text);

private:
    int                     _id                 =0;
    QJsonObject             _config;
    QFormLayout*            _formLayout         =nullptr;
    QComboBox*              _typeCombo          =nullptr;
    QComboBox*              _locationCombo      =nullptr;
    QVector<Argument>       _args;
    QVector<QWidget*>       _editorWidgets;
    QLineEdit*              _nameLineEdit       =nullptr;
    bool                    _isDefaultName      =false;
};

#endif // CONFIGUREWALLETWIDGET_H
