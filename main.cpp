#include <QApplication>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QIcon icon(QDir::currentPath() + ":/matrix-linux.png");

    MainWindow mainWindow;
    mainWindow.resize(1280,720);
    mainWindow.setWindowTitle("Matrix");
    mainWindow.setWindowIcon(icon);
    mainWindow.show();


    return app.exec();
}
