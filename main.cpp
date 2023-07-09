#include <QApplication>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString appDir = QCoreApplication::applicationDirPath();
    QIcon icon(appDir + "/matrix-linux.png");

    MainWindow mainWindow;
    mainWindow.resize(1280,720);
    mainWindow.setWindowTitle("Matrix");
    mainWindow.setWindowIcon(icon);
    mainWindow.show();


    return app.exec();
}
