#include <QPainter>
#include "mainwindow.h"
#include "cameracapture.h"
#include "virtualcamera.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    videoLabel = new QLabel(this);
    videoLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    videoLabel->setScaledContents(true);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(videoLabel);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    virtualCamera = new VirtualCamera(this);
    virtualCamera->setup();

    cameraCapture = new CameraCapture(this);
    videoProcessing = new VideoProcessing(this);
    connect(cameraCapture, &CameraCapture::frameCaptured, videoProcessing, &VideoProcessing::processFrame);
    connect(videoProcessing, &VideoProcessing::imageGenerated, this, &MainWindow::updateImage);
    connect(videoProcessing, &VideoProcessing::imageGeneratedWithoutBlur, virtualCamera, &VirtualCamera::updateVirtualFrame);
    //connect(cameraCapture, &CameraCapture::frameCaptured, this, &MainWindow::updateImage); //test de la camera
}


MainWindow::~MainWindow() {
    cameraCapture->capture.release();
    virtualCamera->stop();
}

void MainWindow::updateImage(const QImage &image)
{
    videoLabel->setPixmap(QPixmap::fromImage(image));
}