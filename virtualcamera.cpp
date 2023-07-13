#include <QTemporaryFile>
#include "virtualcamera.h"

VirtualCamera::VirtualCamera(QObject *parent) : QObject(parent)
{
    QIcon icon("/usr/bin/matrixresources/icons/Matrix.png");
    iconMatrix = icon;
}

void VirtualCamera::setup()
{
    installed = isV4l2LoopbackInstalled();
    enabled = false;
    if (installed)
    {
        if (!isV4l2Enabled()) {
            QMessageBox msgBoxAvailable;
            msgBoxAvailable.setWindowIcon(iconMatrix); // Assuming iconMatrix is a valid QIcon
            msgBoxAvailable.setWindowTitle("Activation de la caméra");
            msgBoxAvailable.setText("Souhaitez-vous activer la caméra virtuelle ?");
            msgBoxAvailable.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBoxAvailable.setDefaultButton(QMessageBox::No);
            int ret = msgBoxAvailable.exec();

            if (ret == QMessageBox::Yes) {
                enableV4l2();
                if (!isV4l2Enabled()) {
                    QMessageBox msgBoxAvailableError(QMessageBox::Critical, "Erreur", "Une erreur s'est produite lors de l'activation de la caméra virtuelle.");
                    msgBoxAvailableError.setWindowIcon(iconMatrix);;
                    msgBoxAvailableError.exec();
                    return;
                }
            } else {
                QMessageBox msgBoxAvailableError(QMessageBox::Critical, "Erreur", "Une erreur s'est produite lors de l'activation de la caméra virtuelle.");
                msgBoxAvailableError.setWindowIcon(iconMatrix);;
                msgBoxAvailableError.exec();
                return;
            }
        }
        if (detectDummyVideoDevice()) {
            configureVirtualCamera();
        }
    }
    else
    {
        QMessageBox msgBoxInstalled(QMessageBox::Critical, "Erreur", "Le module du noyau Linux v4l2loopback-dkms et l'outil v4l2loopback-utils doivent être installés pour utiliser la caméra virtuelle.");
        msgBoxInstalled.setWindowIcon(iconMatrix);;
        msgBoxInstalled.exec();
    }
}

bool VirtualCamera::isV4l2LoopbackInstalled()
{
    QString kernelVersion = QString::fromLocal8Bit(qgetenv("UNAME_R"));
    QString command = QString("find \"/lib/modules/%1\" -name *v4l2loopback*").arg(kernelVersion);

    QProcess process;
    process.start("sh", QStringList() << "-c" << command);
    process.waitForFinished();
    QByteArray output1 = process.readAllStandardOutput();
    process.start("which", QStringList() << "v4l2loopback-ctl");
    process.waitForFinished();
    QByteArray output2 = process.readAllStandardOutput();
    std::cout << output1.toStdString() << std::endl;
    std::cout << output2.toStdString() << std::endl;
    return output1.contains("v4l2loopback") && output2.contains("v4l2loopback-ctl");
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
    QStringList arguments;
    arguments  << "modprobe" << "v4l2loopback";
    processActivateVirtCam.start("pkexec", arguments);
    processActivateVirtCam.waitForFinished();
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
        }       return false;
    } catch (const std::exception& e) {
        std::cerr << "Une exception s'est produite : " << std::endl;
        return false;
    }
}

void VirtualCamera::createScript(const std::string& scriptPath) {
    std::ofstream scriptFile(scriptPath);
    if (scriptFile.is_open()) {
        scriptFile << "#!/bin/sh\n";
        scriptFile << "pkill -9 gst-launch-1.0\n";
        scriptFile << "rmmod v4l2loopback\n";
        scriptFile << "modprobe v4l2loopback\n";
        scriptFile << "v4l2loopback-ctl set-caps \"video/x-raw, format=I420, width=1280, height=720\" " + devicePath + "\n";
        scriptFile.close();

        // Donner les droits d'exécution au script
        std::string chmodCommand = "chmod +x " + scriptPath;
        std::system(chmodCommand.c_str());
    }
}

void VirtualCamera::configureVirtualCamera()
{
    if (!devicePath.empty()) {
        killProcessByPath();
    }
    QMessageBox msgBoxConfigure;
    msgBoxConfigure.setWindowIcon(iconMatrix); // Assuming iconMatrix is a valid QIcon
    msgBoxConfigure.setWindowTitle("Configuration de la caméra");
    msgBoxConfigure.setText("Souhaitez-vous configurer la caméra virtuelle ?");
    msgBoxConfigure.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBoxConfigure.setDefaultButton(QMessageBox::No);
    int buttonClicked = msgBoxConfigure.exec();
    if (buttonClicked == QMessageBox::No)
    {
        return;
    }
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    tempFile = tempDir / "tempScript.sh";
    std::string scriptPath = tempFile;
    createScript(scriptPath);
    QStringList arguments;
    arguments << QString::fromUtf8(scriptPath);
    processPipeline.start("pkexec", arguments);

    if (processPipeline.waitForStarted()) {
        std::cout << "Processus démarré avec succès." << std::endl;

        if (processPipeline.waitForReadyRead()) {
            std::cout << "Processus démarré avec succès." << std::endl;
            QThread::msleep(2000);
            QByteArray output = processPipeline.readAllStandardOutput();
            QString outputString = QString::fromUtf8(output);
            std::cout << "Fin de fichier : " << outputString.toStdString() << std::endl;

            if (outputString.endsWith("NULL...\n") || outputString.endsWith("PAUSED...\n")) {
                handleProcessOutput();
            } else {
                processPipeline.kill();
                std::cout << "Erreur lors du démarrage du processus." << std::endl;
            }
        }
    } else {
        std::cout << "Erreur lors du démarrage du processus." << std::endl;
    }
}

void VirtualCamera::handleProcessOutput() {
    QProcess process;
    QString qstr = QString::fromStdString(devicePath);
    process.start("v4l2-ctl", QStringList() << "--all" << "-d" << qstr);
    process.waitForFinished();
    QString processOutput = process.readAllStandardOutput();

    QStringList lines = processOutput.split('\n');
    QString widthHeight;
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines.at(i);
        if (line.contains("Format Video Output:")) {
            // Recherche de la ligne suivante
            if (i + 1 < lines.size()) {
                QString nextLine = lines.at(i + 1);
                if (nextLine.contains("Width/Height")) {
                    widthHeight = nextLine.mid(21);
                    break;
                }
            }
        }
    }
    QString width;
    QString height;
    if (!widthHeight.isEmpty()) {
        QStringList dimensions = widthHeight.split('/');
        if (dimensions.size() == 2) {
            width = dimensions[0].trimmed();
            height = dimensions[1].trimmed();
            qDebug() << "Largeur : " << width;
            qDebug() << "Hauteur : " << height;
        }
    }

    if (width == "1280" && height == "720") {
        enabled = true;
        QMessageBox msgBoxSuccess;
        msgBoxSuccess.setWindowIcon(iconMatrix);
        msgBoxSuccess.setWindowTitle("Configuration terminée");
        msgBoxSuccess.setText("La caméra virtuelle est configurée avec succès.");
        msgBoxSuccess.setIcon(QMessageBox::Information);
        msgBoxSuccess.exec();
        return;
    }
    QMessageBox msgBoxFailure;
    msgBoxFailure.setWindowIcon(iconMatrix);
    msgBoxFailure.setWindowTitle("Échec de la configuration");
    msgBoxFailure.setText("La configuration de la caméra virtuelle est incorrecte. Vérifiez les dimensions de la caméra.");
    msgBoxFailure.setIcon(QMessageBox::Critical);
    msgBoxFailure.exec();
}

void VirtualCamera::killProcessByPath() {
    FILE* p = popen("pgrep -f v4l2loopback", "r");
    if (p == nullptr) {
        std::cerr << "Erreur lors de l'exécution de pgrep" << std::endl;
        return;
    }
    char pidBuffer[16];
    while (fgets(pidBuffer, sizeof(pidBuffer), p)) {
        size_t len = std::strlen(pidBuffer);
        if (len > 0 && pidBuffer[len - 1] == '\n') {
            pidBuffer[len - 1] = '\0';
        }
        pid_t pid = atoi(pidBuffer);
        if (pid > 0) {
            if (kill(pid, SIGTERM) == 0) {
                std::cout << "Processus avec PID " << pid << " terminé" << std::endl;
            } else {
                std::cerr << "Erreur lors de la terminaison du processus avec PID " << pid << std::endl;
            }
        }
    }
    pclose(p);
}

cv::Mat VirtualCamera::QImageToCvMat(const QImage &image)
{
    cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.constBits()), image.bytesPerLine());
    cv::Mat matRGB;
    cv::cvtColor(mat, matRGB, cv::COLOR_RGBA2RGB);
    return matRGB;
}


void VirtualCamera::updateVirtualFrame(const QImage& image) {
    if (installed && enabled) {
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

void VirtualCamera::stop() {
    QString killCommand = "pkill -9 gst-launch-1.0";
    QProcess killProcess;
    killProcess.startDetached(killCommand);
}