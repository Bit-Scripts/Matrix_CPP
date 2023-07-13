#include "cameracapture.h"

CameraCapture::CameraCapture(QObject *parent)
        : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &CameraCapture::processFrame);
    timer.setInterval(30);

    openCamera();
}

CameraCapture::~CameraCapture()
{
    closeCamera();
}

bool CameraCapture::openCamera()
{
    cameraId = 0;
    frameWidth = 1280;
    frameHeight = 720;
    
    capture.open(cameraId, cv::CAP_V4L2);
    
    if (!capture.isOpened())
    {
        return false;
    }

    capture.set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);

    timer.start();

    return true;
}

void CameraCapture::closeCamera()
{
    capture.release();
    timer.stop();
}

void CameraCapture::processFrame()
{
    cv::Mat frame;
    capture.read(frame);
    cv::Mat resizedImage;
    cv::resize(frame, resizedImage, cv::Size(), 1, 1);

    cv::cvtColor(resizedImage, frame, cv::COLOR_BGR2RGB);
    //frame = resizedImage;



    if (frame.empty()) {
        std::cerr << "Aucune image capturée" << std::endl;
    }
    else
    {
        QImage image(frame.data,
                    frame.cols,
                    frame.rows,
                    frame.step,
                    QImage::Format_RGB888);

        QImage logo = QImage("/usr/bin/matrixresources/icons/MatrixLogo.png");

        QImage resizedLogo = logo.scaled(image.width(), image.height(), Qt::KeepAspectRatio);

        QPainter painter(&image);

        int x = (image.width() - resizedLogo.width()) / 2;
        int y = 0;

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(x, y, resizedLogo);
        painter.end();

        emit frameCaptured(image);
    }
}