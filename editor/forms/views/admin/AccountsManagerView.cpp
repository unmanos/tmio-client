#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <rest/Request.h>

#include "AccountsManagerView.h"
#include "Application.h"

//
#define URL_ACCOUNTS        "admin/accounts"

//
#define COL_USERNAME        0
#define COL_EMAIL           1
#define COL_DISPLAY_NAME    2
#define COL_GROUPS          3
#define COL_COUNT           4

//
AccountsManagerView::AccountsManagerView(QWidget *parent) : QWidget(parent)
{
    setLayout(new QVBoxLayout());
    // setup title frame
    {
        QFrame*             titleFrame=new QFrame();
        QHBoxLayout*        titleLayout=new QHBoxLayout();
        titleFrame->setLayout(titleLayout);
        titleLayout->setMargin(0);
        titleLayout->addSpacing(8);
        // add pair info
        {
            QString rt;
            rt+="<div style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                            "margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:12pt;\">";
            rt+=QString("<span style=\"font-size:11pt;\"><b>%1</b></span>").arg("name");
            //rt+=QString("<span style=\"font-size:9pt;\"> / %1</span>").arg(_quoteTicker);
            rt+="<br>";
            rt+=QString("<span style=\"font-size:8pt;\">%1</span>").arg("symbol");
            rt+="</div>";

            QLabel *label = new QLabel();
            label->setText(rt);
            titleLayout->addWidget(label);
            titleLayout->addStretch(1);
        }
        layout()->addWidget(titleFrame);
    }
    // tree view
    _treeView           =new QTreeView(this);
    _treeView->setSortingEnabled(true);
    _treeView->setAlternatingRowColors(true);
    _treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    _model              =new QStandardItemModel();
    _model->setHorizontalHeaderItem(COL_USERNAME, new QStandardItem(tr("Username")));
    _model->setHorizontalHeaderItem(COL_EMAIL, new QStandardItem(tr("Email")));
    _model->setHorizontalHeaderItem(COL_DISPLAY_NAME, new QStandardItem(tr("Display name")));
    _model->setHorizontalHeaderItem(COL_GROUPS, new QStandardItem(tr("Groups")));

    _treeView->setModel(_model);

    QHeaderView*        listHeader=new QHeaderView(Qt::Orientation::Horizontal);
    listHeader->setSortIndicatorShown(true);
    _treeView->setHeader(listHeader);
    listHeader->setSortIndicator(COL_EMAIL,Qt::SortOrder::AscendingOrder);

    listHeader->setSectionsMovable(true);
    listHeader->resizeSection(COL_USERNAME,150);
    listHeader->resizeSection(COL_EMAIL,250);
    listHeader->resizeSection(COL_DISPLAY_NAME,150);
    listHeader->resizeSection(COL_GROUPS,150);

    _treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    _treeView->setSelectionBehavior(QAbstractItemView::SelectRows);

    _treeView->setUniformRowHeights(true);
    layout()->addWidget(_treeView);
    //
    updateAccounts();
}

//
void                    AccountsManagerView::updateAccounts()
{
    if(!_accountsRequest)
    {
        if(_model->hasChildren())
        {
            _model->removeRows(0, _model->rowCount());
        }
        _accountsRequest=Application::instance().connection().createRequest(URL_ACCOUNTS);
        connect(_accountsRequest, SIGNAL(finished(tmio::rest::RequestResult&)), this, SLOT(onReceiveAccounts(tmio::rest::RequestResult&)));
        _accountsRequest->get(false);
    }
}

//
void                    AccountsManagerView::onReceiveAccounts(tmio::rest::RequestResult& result)
{
    QJsonArray          entries=result._document.array();
    for(int i=0;i<entries.count();i++)
    {
        QJsonObject             account =entries.at(i).toObject();
        QList<QStandardItem*>   row;
        row.append(new QStandardItem(account["username"].toString()));
        row.append(new QStandardItem(account["email"].toObject()["address"].toString()));
        row.append(new QStandardItem(account["displayName"].toString()));

        QString                 groups;
        for(int ig=0;ig<account["groups"].toArray().size();ig++)
        {
            groups+=(groups.length()?", ":"")+account["groups"].toArray().at(ig).toString();
        }
        row.append(new QStandardItem(groups));


        _model->appendRow(row);
    }
    delete(_accountsRequest);
    _accountsRequest=nullptr;
}
