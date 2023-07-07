#include <QTemporaryFile>
#include "virtualcamera.h"

VirtualCamera::VirtualCamera(QObject *parent) : QObject(parent)
{ }

void VirtualCamera::setup()
{
    installed = isV4l2LoopbackInstalled();
    if (installed)
    {
        enabled = isV4l2Enabled();
        if (!enabled)
        {
            enableV4l2();
        }
        enabled = isV4l2Enabled();
        if (!enabled) {
            QMessageBox::critical(nullptr, "Erreur", "Impossible d'activer la caméra virtuelle.");
        }
        else {
            cameraDetected = detectDummyVideoDevice();
            configureVirtualCamera();
        }
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

void VirtualCamera::enableV4l2()
{
    QMessageBox::information(nullptr, "Demande d'authentification", "Le mot de passe d'administration sera demandé pour activer la caméra virtuelle.");

    QProcess process;
    process.setEnvironment(QProcess::systemEnvironment());
    process.start("pkexec", QStringList() << "--user" << "root" << "modprobe" << "v4l2loopback");
    process.waitForFinished();
    if (process.exitCode() == 0)
    {
        // La commande pkexec s'est terminée avec succès
        // Faites le traitement supplémentaire si nécessaire
    }
    else
    {
        // La commande pkexec a échoué, affichez l'erreur
        qDebug() << "Erreur lors de l'exécution de pkexec : ";
    }
    process.close();
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
    arguments << "v4l2loopback-ctl" << "set-fps" << "30" << devicePath.c_str() << "set-caps" << "NV12:640x480" << devicePath.c_str();
    //arguments << "v4l2loopback-ctl" << "set-fps" << "30" << devicePath.c_str() << "set-image" << "MatrixLogo.png" << devicePath.c_str() << "set-caps" << "\"video/x-raw, format=nv12, width=1280, height=720\"" << devicePath.c_str();
    //arguments << "v4l2-ctl" << "-d" << devicePath.c_str() << "--set-fmt-video=width=1280,height=720,pixelformat=nv12";
    QProcess process;
    process.setProgram("pkexec");
    process.setArguments(arguments);
    process.start();
    process.waitForFinished();
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

        QImage resizedImage = image.scaled(640, 480, Qt::IgnoreAspectRatio);
        cv::Mat cvImage = QImageToCvMat(resizedImage);
        cv::Mat cvImageYUV;
        cv::cvtColor(cvImage, cvImageYUV, cv::COLOR_BGRA2YUV_I420);

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


/*void VirtualCamera::updateVirtualFrame(const QImage& image) {
    if (installed && enabled && cameraDetected) {
        const std::string devicePath = "/dev/video4";
        int fd = open(devicePath.c_str(), O_WRONLY);
        if (fd == -1) {
            std::cerr << "Impossible d'ouvrir le périphérique de la caméra virtuelle" << std::endl;
            return;
        }

        QImage resizedImage = image.scaled(1920, 1080, Qt::IgnoreAspectRatio);
        cv::Mat cvImage = QImageToCvMat(resizedImage);
        cv::Mat cvImageRgb24;
        cv::cvtColor(cvImage, cvImageRgb24, cv::COLOR_RGBA2RGB);

        QTemporaryFile tempFile;
        tempFile.setFileTemplate("XXXXXX.png");
        if (tempFile.open()) {
            cv::imwrite(tempFile.fileName().toStdString(), cvImageRgb24);

            QStringList arguments;
            arguments << "-loop" << "1" << "-re" << "-i" << tempFile.fileName() << "-f" << "v4l2" << "-vcodec" << "rawvideo" << "-pixfmt" << "nv12" << "-vf" << "transpose=4" << devicePath.c_str();
            QProcess process;
            process.setProgram("ffmpeg");
            process.setArguments(arguments);

            connect(&process, &QProcess::readyReadStandardOutput, [&process]() {
                QByteArray output = process.readAllStandardOutput();
                // Faire quelque chose avec la sortie standard de ffmpeg
            });

            connect(&process, &QProcess::readyReadStandardError, [&process]() {
                QByteArray error = process.readAllStandardError();
                // Faire quelque chose avec les erreurs de ffmpeg
            });

            process.start();
            process.waitForFinished();

            close(fd);
        } else {
            std::cerr << "Erreur lors de la création du fichier temporaire" << std::endl;
        }
    }
}

        //cv::imwrite("image_rgb24.jpg", cvImageRgb24);

        /*struct v4l2_format format{};
        format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        if (ioctl(fd, VIDIOC_G_FMT, &format) < 0) {
            perror("Impossible d'obtenir le format de la caméra virtuelle");
            close(fd);
            return;
        }

        format.fmt.pix.width = 1280;
        format.fmt.pix.height = 720;
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        format.fmt.pix.sizeimage = cvImageRgb24.total() * cvImageRgb24.elemSize();
        format.fmt.pix.field = V4L2_FIELD_NONE;

        if (ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
            perror("Impossible de définir le format de la caméra virtuelle");
            close(fd);
            return;
        }

        size_t written = write(fd, cvImageRgb24.data, format.fmt.pix.sizeimage);
        if (written == -1) {
            std::cerr << "Erreur lors de l'écriture des données dans la caméra virtuelle" << std::endl;
        }
        close(fd);
    }
}*/
