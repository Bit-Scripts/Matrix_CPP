#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "videoprocesssing.h"
#include "virtualcamera.h"

VideoProcessing::VideoProcessing(QObject *parent) : QObject(parent)
{

    QString appDir = QCoreApplication::applicationDirPath();
    int fontId = QFontDatabase::addApplicationFont(appDir + "/fonts/mtx.ttf");

    if (fontId == -1) {
        std::cerr << "Erreur dans VideoProcessing : Police mtx non chargée" << std::endl;
        return;
    }

    QString matrix = QFontDatabase::applicationFontFamilies(fontId).at(0);
    mtx = QFont(matrix, 12);

    characters = L" úù_.-,`;:ö+/'=Åa~^³xwvusc*><onø÷\"ÖÈÊe|ÄrÃ±iÉzm)(¹l¡»·ãI}{][ýÙÐ§1jÔÍ¾ÂætL!ÌÁÀº¸´Ëy?Òþ¶qg¨p2ÑÏ¼²TfCJäÝ7Óü×©3YØÎ°Õ½«5Zóõ¿Ç£ôíSÆµ6¬¢kdûbîàªF4òXhÚ¯9êP$#GáßEÛA¤VðïU&éOÞåKD®8âHÜìRBëQW¦0@ñMèNç¥";
    //characters = " úù_.-,`;:ö+/'=Åa~^³xwvu%sc*><onø÷\"ÖÈÊe|ÄrÃ±iÉzm)(¹l¡»·ãI}{][ýÙÐ§1jÔÍ¾ÂætL!ÌÁÀº¸´Ëy?Òþ¶qg¨p2ÑÏ¼²TfCJäÝ7Óü×©3YØÎ°Õ½«5Zóõ¿Ç£ôíSÆµ6¬¢kdûbîàªF4òXhÚ¯9êP$#GáßEÛA¤VðïU&éOÞåKD®8âHÜìRBëQW¦0@ñMèNç¥";

    textEdit = new QTextEdit;
    textEdit->setReadOnly(true);

    len_array_width = 640;
    len_array_height = 480;

    QFontMetrics fontMetrics(mtx);
    int characterWidth = fontMetrics.averageCharWidth();
    int characterHeight = fontMetrics.height();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist1(5, 20);
    std::uniform_int_distribution<int> dist2(1, 3);
    std::uniform_int_distribution<int> dist3(0, 4);


    for (int i = 0; i <= 1280; i+=characterWidth) {
        int x = i;
        int y = dist3(gen);
        int size = dist1(gen) * characterHeight;
        int speed = dist2(gen) * characterHeight;
        int maxTrailLength  = speed * size / characterHeight;

        Raindrop* raindrop = new Raindrop(x, y, size, speed, maxTrailLength);
        raindrops.push_back(raindrop);
    }

    imageWithAscii = QImage(1280, 720, QImage::Format_ARGB32);
    imageWithAscii.fill(Qt::black);
}

void VideoProcessing::processFrame(const QImage &camera_frame)
{
    update_ascii_image(camera_frame);
}

std::pair<std::vector<std::wstring>, std::vector<wchar_t>> VideoProcessing::image_to_ascii(const cv::Mat& image)
{
    try
    {
        if (image.empty()) {
            // l'image est vide, donc il n'y a rien à dessiner
            return std::make_pair(std::vector<std::wstring>(), std::vector<wchar_t>());
        }

        int totalCharacters = characters.length();

        QFontMetrics fontMetrics(mtx);

        int characterWidth = fontMetrics.averageCharWidth();
        int characterHeight = fontMetrics.height();

        int numRectanglesWidth = image.cols / characterWidth;
        int numRectanglesHeight = image.rows / characterHeight;

        std::vector<wchar_t> asciiCharacters;
        asciiCharacters.reserve(numRectanglesWidth * numRectanglesHeight);

        std::vector<std::wstring> ascii_art;
        ascii_art.reserve(numRectanglesHeight);

        for (int i = 0; i < numRectanglesHeight; ++i)
        {
            std::wstring ascii_row;
            ascii_row.reserve(numRectanglesWidth);

            for (int j = 0; j < numRectanglesWidth; ++j)
            {
                cv::Mat square = image(cv::Rect(j * characterWidth, i * characterHeight, characterWidth, characterHeight));
                cv::Scalar mean = cv::mean(square);
                int totalRGB = static_cast<int>(mean[0] + mean[1] + mean[2]);
                int index;
                if (totalRGB >= 0) {
                    index = totalRGB * (totalCharacters - 1) / 765;
                } else {
                    index = 0;
                }

                asciiCharacters.push_back(characters[index]);
                ascii_row += characters[index];
            }

            ascii_art.push_back(ascii_row);
        }

        return std::make_pair(ascii_art, asciiCharacters);

    }
    catch (const std::exception &e)
    {
        std::cerr << "Erreur dans image_to_ascii : " << e.what() << std::endl;
        return std::make_pair(std::vector<std::wstring>(), std::vector<wchar_t>());
    }
}

void VideoProcessing::update_ascii_image(const QImage &image)
{
    try {
        cv::Mat cvImage;
        if (image.format() == QImage::Format_ARGB32 || image.format() == QImage::Format_RGB888) {
            cvImage = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar *>(image.constBits()),
                              image.bytesPerLine()).clone();
        } else {
            qDebug() << "Incompatible image format";
            return;
        }

        auto ascii_image = image_to_ascii(cvImage);

        const auto &lines = ascii_image.first;
        const auto &colors = ascii_image.second;

        imageWithAscii.fill(Qt::transparent);

        if (ascii_image.first.empty()) {
            // ascii_image est vide, donc il n'y a rien à dessiner
            return;
        }

        imageWithAscii.fill(Qt::transparent);

        QPainter painter(&imageWithAscii);
        QColor textColor(QColor("#004400"));
        QPen textPen(textColor);
        painter.setFont(mtx);
        painter.setPen(textPen);

        QFontMetrics fontMetrics(mtx);
        int lineHeight = fontMetrics.height();

        int y = 0;

        std::vector<QString> ascii_image_qstring;

        for (const auto &line: lines) {
            ascii_image_qstring.push_back(QString::fromStdWString(line));
        }

        for (const QString &row: ascii_image_qstring) {
            painter.drawText(0, y, image.width(), lineHeight, Qt::AlignLeft, row);
            y += lineHeight;
        }

        paintRainDrop(painter, ascii_image);

        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(1.0);
        imageWithAscii2 = applyEffectToImage(imageWithAscii, blur);

        painter.end();

        QImage finalImage(imageWithAscii.size(), QImage::Format_ARGB32);
        QPainter painter2(&finalImage);
        painter2.drawImage(0, 0, imageWithAscii2);
        painter2.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter2.drawImage(0, 0, imageWithAscii);

        painter2.end();

        emit imageGenerated(finalImage);
        emit imageGeneratedWithoutBlur(imageWithAscii);
    } catch (const std::logic_error& e) {
        std::cout << "Une exception s'est produite lors du dessin de l'ascii_art' : " << e.what() << std::endl;
    }
}

QImage VideoProcessing::applyEffectToImage(QImage src, QGraphicsEffect *effect)
{
    if(src.isNull()) return QImage();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size()+QSize(0, 01), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF( 0, 0, src.width(), src.height()));
    return res;
}

void VideoProcessing::paintRainDrop(QPainter& painter, std::pair<std::vector<std::wstring>, std::vector<wchar_t>> ascii_image)
{
    try {
        if (ascii_image.first.empty()) {
            return;
        }
        QRect imageRect(0, 0, 1280, 720);
        painter.fillRect(imageRect, Qt::transparent);

        QFontMetrics fontMetrics(mtx);

        int characterWidth = std::abs(fontMetrics.averageCharWidth());
        int lineHeight = std::abs(fontMetrics.height());

        const std::vector<std::wstring> &asciiRows = ascii_image.first;

        for (auto it = raindrops.begin(); it != raindrops.end();) {
            Raindrop *raindrop = *it;
            int raindropX = raindrop->getX();
            int raindropY = raindrop->getY();
            int raindropLength = raindrop->getSize();
            raindrop->setImageHeight(imageWithAscii.height());

            if (raindropY < 0 || raindropY >= imageWithAscii.height()) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dist1(15, 25);
                std::uniform_int_distribution<int> dist2(1, 3);
                std::uniform_int_distribution<int> dist3(0, 30);

                raindrop->setX(raindropX);
                raindrop->setY(dist3(gen));
                int newSize = dist1(gen) * lineHeight;
                raindrop->setSize(newSize);
                int newSpeed = dist2(gen) * lineHeight;
                int newMaxTrailLength = newSpeed * newSize / lineHeight;
                raindrop->setMaxTrailLength(newMaxTrailLength);

                if (characterWidth * raindrops.size() < imageWithAscii.width()){
                    int x2 = raindropX;
                    int y2 = (imageWithAscii.height() / 2) + dist3(gen);
                    int size2 = dist1(gen) * lineHeight;
                    int speed2 = dist2(gen) * lineHeight;
                    int maxTrailLength2 = newSpeed * newSize / lineHeight;
                    Raindrop *newRaindrop = new Raindrop(x2, y2, size2, speed2, maxTrailLength2);
                    raindrops.push_back(newRaindrop);
                }
                continue;
            }

            const std::wstring &asciiRow = asciiRows[raindropY / lineHeight];
            wchar_t asciiCharacter = asciiRow[raindropX / characterWidth];
            int startColor = 255;
            int endColor = 68;
            int charactersTrain = raindropLength / lineHeight;
            int colorDifference = std::abs(endColor - startColor);
            int colorCharacters = 1;
            if (colorDifference > 0) {
                qreal colorCharactersReal = static_cast<qreal>(lineHeight) * static_cast<qreal>(charactersTrain) /
                                            static_cast<qreal>(colorDifference);
                colorCharacters = colorCharactersReal;
            }
            if (colorCharacters == 0) {
                colorCharacters = 1;
            }
            int colorStep = colorDifference / charactersTrain;

            int currentColor = startColor;

            std::vector<Raindrop *> newRaindrops;
            qreal opacity = static_cast<int>(raindrop->getOpacity() * 255);
            for (int i = 0; i < raindropLength; i += colorCharacters * lineHeight) {
                for (int j = i; j < i + colorCharacters * lineHeight; j += lineHeight) {
                    int charColor = currentColor;
                    if (raindropX >= 0 && raindropX < imageWithAscii.width() && raindropY - j >= 0 &&
                        raindropY - j < imageWithAscii.height()) {
                        painter.setPen(QColor(0, charColor, 0, opacity));
                        painter.drawText(raindropX, raindropY - j, characterWidth, lineHeight, Qt::AlignLeft,
                                         QString::fromStdWString(std::wstring(1, asciiCharacter)));
                    }
                }
                currentColor -= colorStep;
            }

            if (raindropX >= 0 && raindropX < imageWithAscii.width() && raindropY >= 0 &&
                raindropY < imageWithAscii.height()) {
                painter.setPen(QColor(255, 255, 255, opacity));
                painter.drawText(raindropX, raindropY, characterWidth, lineHeight, Qt::AlignLeft,
                                 QString::fromStdWString(std::wstring(1, asciiCharacter)));
            }

            raindrop->update();
            ++it;
        }
    } catch (const std::logic_error& e) {
        std::cout << "Une exception s'est produite lors du dessin de la goutte de pluie : " << e.what() << std::endl;
    }
}

