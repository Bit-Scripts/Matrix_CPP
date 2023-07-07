#ifndef MATRIX_VIRTUALCAMERA_H
#define MATRIX_VIRTUALCAMERA_H

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <string>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <QImage>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <opencv2/opencv.hpp>
#include "mainwindow.h"
#include "videoprocesssing.h"

class VirtualCamera : public QObject
{
Q_OBJECT

public:
    VirtualCamera(QObject *parent = nullptr);
    QProcess process;

public slots:
    void setup();
    bool isV4l2LoopbackInstalled();
    bool isV4l2Enabled();
    void updateVirtualFrame(const QImage& image);
    void configureVirtualCamera();
    void stop();

private:
    bool detectDummyVideoDevice();
    std::string devicePath;
    bool installed;
    bool enabled;
    bool cameraDetected;
    cv::Mat QImageToCvMat(const QImage &image);
    QProcess processPipeline;
};

#endif // VIRTUALCAMERA_H
