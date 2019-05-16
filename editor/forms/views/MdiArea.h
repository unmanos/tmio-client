#ifndef MDIAREA_H
#define MDIAREA_H
#include <QMdiArea>
#include <QMenu>

namespace Ui {
class MdiArea;
}

class MdiArea : public QMdiArea
{
    Q_OBJECT

public:
    explicit MdiArea(QMenu* menu,QWidget *parent = 0);
    ~MdiArea();

    bool        activateView(QString viewID);
    bool        createView(QAction*);

public slots:
    void        onSubWindowActivated(QMdiSubWindow*);
    void        onMenuAction();

signals:


private:
    Ui::MdiArea *ui;
    QMenu*      _viewMenu           =nullptr;

};

#endif // MDIAREA_H
