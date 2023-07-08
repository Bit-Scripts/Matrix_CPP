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
#include <QThread>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <cstdlib>
#include <signal.h>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "mainwindow.h"
#include "videoprocesssing.h"

class VirtualCamera : public QObject
{
Q_OBJECT

public:
    VirtualCamera(QObject *parent = nullptr);
    QProcess process;
    QProcess processPipeline;

public slots:
    void setup();
    bool isV4l2LoopbackInstalled();
    bool isV4l2Enabled();
    void updateVirtualFrame(const QImage& image);
    void configureVirtualCamera();
    void enableV4l2();
    void stop();

private:
    bool detectDummyVideoDevice();
    std::string devicePath;
    bool installed;
    bool enabled;
    bool cameraDetected;
    QString pidFileName;
    cv::Mat QImageToCvMat(const QImage &image);
    QProcess processActivateVirtCam;
    QProcess checkProcess;
    QString outputFileName;
    std::filesystem::path tempFile;
    void killProcessByPath();
    void createScript(const std::string& scriptPath);
    void handleProcessOutput();
};

#endif // VIRTUALCAMERA_H
