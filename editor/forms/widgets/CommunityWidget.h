#ifndef SOCIALWIDGET_H
#define SOCIALWIDGET_H
#include <QMap>
#include <QWidget>

//
class QTabWidget;

//
class CommunityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CommunityWidget(QWidget *parent = nullptr);

    void                    openAsset(const QString& assetID);
    void                    removeUnpinnedTabs();


signals:

public slots:
    void                    onTabCloseRequested(int index);


private:
    QTabWidget*             _tabWidget      =nullptr;
    QMap<QString,QWidget*>  _pages;
};

#endif // SOCIALWIDGET_H
