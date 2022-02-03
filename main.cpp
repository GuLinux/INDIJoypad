#include "mainwindow.h"
#include "settings.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(SETTINGS_ORG_NAME);
    QCoreApplication::setApplicationName("INDIJoypad");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    MainWindow w;
    w.show();
    return a.exec();
}
