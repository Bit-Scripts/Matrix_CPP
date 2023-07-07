#ifndef RAINDROP_H
#define RAINDROP_H
#include <vector>
#include <QPoint>


class Raindrop {
public:
    Raindrop(int x, int y, int size, int speed, int maxTrailLength);

    void update();
    int getX() const;
    int getY() const;
    int getSize() const;
    int getMaxTrailLength() const;
    qreal getOpacity() const;
    std::vector<QPoint> previousPositions;
    int maxTrailLength;

    void setX(int new_x);
    void setY(int new_y);
    void setSize(int new_size);
    void setMaxTrailLength(int new_maxTrailLength);
    void setImageHeight(int new_imageHeight);

private:
    int x;
    int y;
    int size;
    int speed;
    qreal opacity;
    int imageHeight;
    qreal fadeDurationFactor;
};

#endif // RAINDROP_H

