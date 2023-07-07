#include <QTemporaryFile>
#include "virtualcamera.h"

VirtualCamera::VirtualCamera(QObject *parent) : QObject(parent)
{ }

void VirtualCamera::setup()
{
    installed = isV4l2LoopbackInstalled();
    if (installed)
    {
        enabled = false;
        configureVirtualCamera();
    }
    else
    {
        QMessageBox::critical(nullptr, "Erreur", "Le module du noyau Linux v4l2loopback-dkms et l'outils v4l2loopback-utils doivent être installés pour utiliser la caméra virtuelle.");
    }
}
bool VirtualCamera::isV4l2LoopbackInstalled()
{
    QProcess process;
    process.start("dkms", QStringList() << "status");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    return output.contains("v4l2loopback");
}

bool VirtualCamera::isV4l2Enabled()
{
    QProcess process;
    process.start("bash", QStringList() << "-c" << "lsmod | grep v4l2loopback");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    bool isEnabled = !output.isEmpty();
    return isEnabled;
}

bool VirtualCamera::detectDummyVideoDevice()
{
    try {
        for (int index = 0; index < 10; ++index) {
            devicePath = "/dev/video" + std::to_string(index);
            int fd = open(devicePath.c_str(), O_RDONLY);
            if (fd == -1) {
                continue;  // Passe à l'itération suivante si le périphérique n'existe pas
            }

            struct v4l2_capability cap;
            if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
                close(fd);
                continue;  // Passe à l'itération suivante en cas d'erreur de requête des capacités
            }

            std::string deviceName(reinterpret_cast<char *>(cap.card));
            if (deviceName == "Dummy video device (0x0000)") {
                std::cout << "Caméra virtuelle détectée : " << deviceName << " (index : " << index << ")" << std::endl;
                close(fd);
                return true;
            }

            close(fd);
        }       return false;  // Caméra virtuelle non détectée
    } catch (const std::exception& e) {
        std::cerr << "Une exception s'est produite : " << std::endl;
        return false;
    }

}

void VirtualCamera::configureVirtualCamera()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Confirmation", "Voulez-vous configurer la caméra virtuelle ?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No)
    {
        return;
    }

    QStringList arguments;
    QString command;
    command = "/usr/bin/pkexec";
    if (isV4l2Enabled()) {
        arguments << "sh" << "-c" << "pkexec sh -c 'rmmod v4l2loopback && modprobe v4l2loopback && v4l2loopback-ctl set-caps \"video/x-raw, format=I420, width=1280, height=720\" " + QString(devicePath.c_str()) + "'";
    } else {
        arguments << "sh" << "-c" << "pkexec sh -c 'modprobe v4l2loopback && v4l2loopback-ctl set-caps \"video/x-raw, format=I420, width=1280, height=720\" " + QString(devicePath.c_str()) + "'";
    }
    processPipeline.startDetached(command, arguments);
    QTimer::singleShot(15000, [=]() {
        enabled = true;
        cameraDetected = detectDummyVideoDevice();
    });
}


cv::Mat VirtualCamera::QImageToCvMat(const QImage &image)
{
    cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
    cv::Mat matRGB;
    cv::cvtColor(mat, matRGB, cv::COLOR_RGBA2RGB);
    return matRGB;
}


void VirtualCamera::updateVirtualFrame(const QImage& image) {
    if (installed && enabled && cameraDetected) {
        const std::string devicePath = "/dev/video4";
        int fd = open(devicePath.c_str(), O_WRONLY);
        if (fd == -1) {
            std::cerr << "Impossible d'ouvrir le périphérique de la caméra virtuelle" << std::endl;
            return;
        }

        cv::Mat cvImage = QImageToCvMat(image);
        cv::Mat cvImageYUV;
        cv::cvtColor(cvImage, cvImageYUV, cv::COLOR_RGB2YUV_I420);

        struct v4l2_format vid_format;
        vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        if (ioctl(fd, VIDIOC_G_FMT, &vid_format) == -1) {
            std::cerr << "Erreur lors de la récupération du format de la caméra virtuelle" << std::endl;
            close(fd);
            return;
        }

        vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
        vid_format.fmt.pix.bytesperline = cvImageYUV.cols;
        vid_format.fmt.pix.sizeimage = cvImageYUV.total() + (cvImageYUV.total() / 2);
        if (ioctl(fd, VIDIOC_S_FMT, &vid_format) == -1) {
            std::cerr << "Erreur lors de la configuration du format de la caméra virtuelle" << std::endl;
            close(fd);
            return;
        }

        size_t written = write(fd, cvImageYUV.data, cvImageYUV.total() + (cvImageYUV.total() / 2));
        if (written == -1) {
            std::cerr << "Erreur lors de l'écriture des données dans la caméra virtuelle" << std::endl;
        }

        close(fd);
    }
}

void VirtualCamera::stop()
{
    if (processPipeline.state() != QProcess::NotRunning)
    {
        processPipeline.kill();
        processPipeline.waitForFinished();
    }
}