#include "Application.h"
#include "MainWindow.h"
#include "forms/system/LoginDialog.h"
#include "forms/system/LoadingDialog.h"

#include <QGuiApplication>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>

#include <Core.h>

#define TAG "main"

#define ACTION_QUIT     0
#define ACTION_START    1

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //QtWebView::initialize();
    Application::instance().start();
    TM_TRACE(TAG, "Startint chocolates factory...");

    // windows, oxygen, cleanlooks
    /*QStringList list=QStyleFactory::keys ();
    fprintf(stderr, "Available styles: \r\n");
    for(int i=0;i<list.size();i++)
    {
        fprintf(stderr, list.at(i).toStdString().c_str());
        fprintf(stderr, "\r\n");
    }*/

    ///
    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    //qApp->setStyle(QStyleFactory::create("fusion"));

    MainWindow* w=NULL;
    LoginDialog dlg;
    dlg.setModal(true);
    int res=dlg.exec();
    if(res==QDialog::Accepted)
    {
        LoadingDialog loadingDlg;
        loadingDlg.setModal(true);
        if(loadingDlg.exec()==QDialog::Accepted)
        {
            // show main window
            w=Application::instance().mainWindow();
            w->showMaximized();
            //w->resize(1920,1080);
            w->show();
        }
        else
        {
            return(1);
        }
    }
    else
    {
        return(1);
    }
    int result=app.exec();
    /*if(w)
    {
        delete(w);
    }*/
    return(result);
}
