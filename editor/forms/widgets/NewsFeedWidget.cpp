#include <QGridLayout>
#include <QTextDocument>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QDesktopServices>
#include "Application.h"
#include "MainWindow.h"
#include "NewsFeedWidget.h"
#include "libs/ui/sort/GenericSortFilterProxyModel.h"

#define ICON_SIZE           48
#define ICON_MARGIN         5

#define COL_CONTENTS        0
#define COL_TIME            1

#define USER_ROLE_HAVE_ICON (Qt::UserRole+1)
#define USER_ROLE_URL       (Qt::UserRole+2)
#define USER_ROLE_TITLE     (Qt::UserRole+3)

//
void NewsFeedWidget::HTMLDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    //options.icon=QIcon();
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;
    // Highlighting text if item is selected
    if (options.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, options.palette.color(QPalette::Active, QPalette::HighlightedText));

    painter->translate(options.rect.left(), options.rect.top());


    //index.data(Qt::Icon)
    //option.icon

    bool haveIcon        =index.data(USER_ROLE_HAVE_ICON).toBool();
    //bool haveIcon        =option.icon.isNull()?false:true;
    int iconWidth        =haveIcon?(ICON_MARGIN*2+ICON_SIZE):0;
    QRect clip(0, 0, options.rect.width()-iconWidth, options.rect.height());
    painter->translate(QPointF(iconWidth,0));
    doc.setTextWidth(options.rect.width()-iconWidth);

    //painter->setClipRect(clip);
    //doc.setDocumentLayout(&ctx);
    painter->setClipRect(clip);
    doc.documentLayout()->draw(painter, ctx);

    //doc.drawContents(painter, clip);

    painter->restore();
}

//
QSize NewsFeedWidget::HTMLDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    //return QSize(doc.idealWidth(), doc.size().height());
    return QSize(static_cast<int>(doc.idealWidth()), ICON_SIZE+ICON_MARGIN*2);
}

//
NewsFeedWidget::NewsFeedWidget(QWidget *parent) : QWidget(parent)
{
    nws::Feed*      feed=Application::instance().newsFeed();

    QGridLayout*    layout=new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    _tree=new QTreeView();
    _model=new QStandardItemModel();
    _tree->setModel(_model);
    _tree->setHeaderHidden(true);
    //_logsWidget->setReadOnly(true);
    _tree->setSortingEnabled(true);
    _tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _tree->setSelectionMode(QAbstractItemView::SingleSelection);
    _tree->setRootIsDecorated(false);

    GenericSortFilterProxyModel*    sortFilter          =new GenericSortFilterProxyModel(_tree);
    _model=new QStandardItemModel();
    _model->setHorizontalHeaderItem(COL_CONTENTS, new QStandardItem(tr("Contents")));
    _model->setHorizontalHeaderItem(COL_TIME, new QStandardItem(tr("Time")));
    sortFilter->setSourceModel(_model);
    _tree->setModel(sortFilter);

    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(false);
    _tree->setHeader(listHeader);

    listHeader->setSortIndicator(COL_TIME,Qt::SortOrder::DescendingOrder);

    //listHeader->setSectionsMovable(false);
    listHeader->setSectionsClickable(false);
    listHeader->setStretchLastSection(false);
    //listHeader->hideSection(COL_ID);
    //listHeader->resizeSection(COL_CONTENTS,200);
    //listHeader->
    listHeader->resizeSection(COL_TIME,70);
    listHeader->resizeSection(COL_CONTENTS,200);
    listHeader->setSectionResizeMode(COL_CONTENTS,QHeaderView::Stretch);
    listHeader->hide();


    layout->addWidget(_tree);

    //_model->appendRow(new QStandardItem("toto"));
    QList<nws::Feed::Item>      items=feed->items();
    foreach(nws::Feed::Item i,items)
    {
        //_model->appendRow(new QStandardItem(i.title));
        onNewsUpdated(i);
    }

    connect(feed,&nws::Feed::newsUpdated,this,&NewsFeedWidget::onNewsUpdated);
    _tree->setIconSize(QSize(ICON_SIZE,ICON_SIZE));

    connect(_tree,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onOpenNews(QModelIndex)));

    // create custom delegate
    HTMLDelegate* delegate = new HTMLDelegate();
    // set model and delegate to the treeview object
    _tree->setItemDelegate(delegate);

}

//
void                NewsFeedWidget::onNewsUpdated(const nws::Feed::Item& i)
{
    // https://stackoverflow.com/questions/1956542/how-to-make-item-view-render-rich-html-text-in-qt
    // contents
    QStandardItem*  item=new QStandardItem();
    QString         html=QString("<b>%1</b> - <i>%2</i><br>%3").arg(i.title).arg(i.feedTitle).arg(i.description);
    item->setData(html,Qt::DisplayRole);
    item->setData(QVariant(i.icon.isNull()?false:true),USER_ROLE_HAVE_ICON);
    item->setData(i.link,USER_ROLE_URL);
    item->setData(i.title,USER_ROLE_TITLE);
    item->setIcon(i.icon);
    item->setToolTip(i.link);
    // time
    QStandardItem*  time    =new QStandardItem();
    QDateTime       newsTime=i.pubDate.toLocalTime();
    QString         dateStr;
    if(newsTime.date()==QDateTime::currentDateTime().date())
    {
        dateStr+="today";
    }
    else
    {
        dateStr+=newsTime.date().toString();
    }
    dateStr+="\r\n"+newsTime.time().toString();
    time->setText(dateStr);
    time->setData(i.pubDate.toSecsSinceEpoch(),USER_ROLE_SORT_NUMERIC);
    // append row
    QList<QStandardItem*>   row;
    row.append(item);
    row.append(time);
    _model->appendRow(row);
}

//
void                NewsFeedWidget::onOpenNews(QModelIndex index)
{
    QModelIndex             firstIndex  =index.sibling(index.row(),0);
    QString                 url         =firstIndex.data(USER_ROLE_URL).toString();
    QDesktopServices::openUrl(url);
}
