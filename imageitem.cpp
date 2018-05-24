#include "imageitem.h"

ImageItem::ImageItem(QQuickItem *parent)
: QQuickPaintedItem(parent)
{
    this->mImage = QImage(":/images/no_image.png");
}

void ImageItem::paint(QPainter *painter) {
    QRectF bounds = boundingRect();
    QImage scaled = this->mImage.scaledToHeight(bounds.height());
    QPointF center = bounds.center() - scaled.rect().center();

    if(center.x() < 0)
        center.setX(0);
    if(center.y() < 0)
        center.setY(0);

   painter->drawImage(center, scaled);
}

void ImageItem::setImage(const QImage &image) {
    this->mImage = image;
    update();

    emit imageChanged();
    emit dimsChanged(QVector2D(mImage.width(), mImage.height()));
}
