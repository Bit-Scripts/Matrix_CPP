#ifndef VIDEOTREATMENT_H
#define VIDEOTREATMENT_H

#include <random>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QFontDatabase>
#include <QLabel>
#include <QObject>
#include <QTextEdit>
#include <QLayout>
#include <QFont>
#include <QResource>
#include <QString>
#include <QPainter>
#include <iostream>
#include <QDir>
#include <limits.h> 
#include <locale>
#include <codecvt>
#include <string>
#include <QColor>
#include <QPen>
#include <QLinearGradient>
#include <utility>
#include <QPoint>
#include <QGraphicsBlurEffect>
#include "mainwindow.h"
#include "raindrop.h"

class VideoProcessing : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessing(QObject *parent = nullptr);
    std::pair<std::vector<std::wstring>, std::vector<wchar_t>> image_to_ascii(const cv::Mat& image);

public slots:
    void processFrame(const QImage &camera_frame);
    QImage applyEffectToImage(QImage src, QGraphicsEffect *effect);

signals:
    void imageGenerated(const QImage& image);
    void imageGeneratedWithoutBlur(const QImage& image);

private:
    QLabel* label;
    int len_array_width;
    int len_array_height;
    std::wstring characters;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    QFont mtx;
    std::map<int, std::wstring> ascii_cache;
    QTextEdit *textEdit;
    QImage textImage;
    QLabel *videoLabel;
    class VideoProcessing *videoTreatmentObject;
    std::vector<Raindrop*> raindrops;
    QImage imageWithAscii;
    QImage imageWithAscii2;

    void update_ascii_image(const QImage &image);
    void paintRainDrop(QPainter& painter, std::pair<std::vector<std::wstring>, std::vector<wchar_t>> ascii_image);
};

#endif // VIDEOTREATMENT_H
