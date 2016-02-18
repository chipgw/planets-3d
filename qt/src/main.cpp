#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    a.setOrganizationName("chipgw");
    a.setApplicationName("Planets3D");
    a.setApplicationVersion(version::git_revision);

    QSurfaceFormat format;
#ifdef NDEBUG
    format.setSwapInterval(1);
#else
    format.setSwapInterval(0);
#endif
    format.setDepthBufferSize(32);
    format.setSamples(32);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow w;
    w.show();

    return a.exec();
}
