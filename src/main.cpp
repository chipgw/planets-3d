#include "mainwindow.h"
#include <QApplication>
#include <QTime>
#include "version.h"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    qsrand(QTime::currentTime().msec());

    a.setOrganizationName("chipgw");
    a.setApplicationName("Planets3D");
    a.setApplicationVersion(version::getVersionString());

    MainWindow w;
    w.showMaximized();

    return a.exec();
}
