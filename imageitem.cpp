#include "imageitem.h"

ImageItem::ImageItem(QQuickItem *parent)
: QQuickPaintedItem(parent)
{
    mImage = QImage(":/images/no_image.png");
}

void ImageItem::paint(QPainter *painter) {
    if (mImage.isNull())
        return;

    QRectF bounds = boundingRect();
    QImage scaled = mImage.scaledToHeight(bounds.height());
    QPointF center = bounds.center() - scaled.rect().center();

    if(center.x() < 0)
        center.setX(0);
    if(center.y() < 0)
        center.setY(0);

   painter->drawImage(center, scaled);
}

void ImageItem::setImage(const QImage &image) {
    mImage = image;
    update();

    emit imageChanged();
    emit dimsChanged(QVector2D(mImage.width(), mImage.height()));
}
