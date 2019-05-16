#ifndef NEWSFEEDWIDGET_H
#define NEWSFEEDWIDGET_H
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <nws/Feed.h>

//
class NewsFeedWidget : public QWidget
{
    Q_OBJECT
public:

    class HTMLDelegate : public QStyledItemDelegate
    {
        void    paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const;
        QSize   sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    };


    explicit NewsFeedWidget(QWidget *parent = nullptr);

signals:

public slots:
    void                onNewsUpdated(const tmio::nws::Feed::Item& i);
    void                onOpenNews(QModelIndex);

private:
    QTreeView*          _tree           =nullptr;
    QStandardItemModel* _model          =nullptr;
};

#endif // NEWSFEEDWIDGET_H


