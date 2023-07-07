#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QLabel>
#include <QImage>
#include <QProcess>
#include <opencv2/opencv.hpp>
#include "cameracapture.h"
#include "videoprocesssing.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateImage(const QImage &image);

private slots:

private:
    Ui::MainWindow ui;
    QLabel *videoLabel;
    class CameraCapture *cameraCapture;
    class VideoProcessing *videoProcessing;
    class VirtualCamera *virtualCamera;
};

#endif // MAINWINDOW_H
