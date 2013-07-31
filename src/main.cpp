#include "mainwindow.h"
#include <QApplication>
#include <QTime>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    qsrand(QTime::currentTime().msec());

    MainWindow w;
    w.showMaximized();

    return a.exec();
}
