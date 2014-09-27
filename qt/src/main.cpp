#include "mainwindow.h"
#include "version.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    a.setOrganizationName("chipgw");
    a.setApplicationName("Planets3D");
    a.setApplicationVersion(version::git_revision);

    MainWindow w;
    w.show();

    return a.exec();
}
