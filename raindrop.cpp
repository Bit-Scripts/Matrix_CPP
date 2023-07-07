#include "raindrop.h"

Raindrop::Raindrop(int x, int y, int size, int speed, int maxTrailLength)
        : x(x), y(y), size(size), speed(speed) , maxTrailLength(maxTrailLength)
{
    imageHeight = 720;
    fadeDurationFactor = .75;
}

void Raindrop::update()
{
    y += speed;
    opacity = 1.0 - (static_cast<qreal>(getY()) / static_cast<qreal>(imageHeight)) * fadeDurationFactor;
    previousPositions.push_back(QPoint(x, y));
    if (previousPositions.size() > maxTrailLength)
    {
        previousPositions.erase(previousPositions.begin());
    }
}

int Raindrop::getX() const
{
    return x;
}

int Raindrop::getY() const
{
    return y;
}

int Raindrop::getSize() const
{
    return size;
}

int Raindrop::getMaxTrailLength() const
{
    return maxTrailLength;
}

qreal Raindrop::getOpacity() const
{
    return opacity;
}

void Raindrop::setX(int new_x)
{
    y = new_x;
}

void Raindrop::setY(int new_y)
{
    y = new_y;
}

void Raindrop::setSize(int new_size)
{
    size = new_size;
}

void Raindrop::setMaxTrailLength(int new_maxTrailLength)
{
    maxTrailLength = new_maxTrailLength;
}

void Raindrop::setImageHeight(int new_imageHeight)
{
    imageHeight = new_imageHeight;
}