#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QCamera>
#include <QImageCapture>
#include <QMediaRecorder>
#include <QTimer>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QDir>
#include <QImage>
#include <QPainter>
#include "videoprocesssing.h"
#include <opencv2/opencv.hpp>

class CameraCapture : public QObject
{
Q_OBJECT

public:
    explicit CameraCapture(QObject *parent = nullptr);
    ~CameraCapture();

    bool openCamera();
    void closeCamera();
    cv::VideoCapture capture;
signals:
    void frameCaptured(const QImage &image);

private slots:
    void processFrame();

private:
    QTimer timer;
    int cameraId;
    int frameWidth;
    int frameHeight;
};

#endif // CAMERA_H
