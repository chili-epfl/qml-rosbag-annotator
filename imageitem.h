#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QQuickPaintedItem>
#include <QQuickItem>
#include <QPainter>
#include <QImage>
#include <QVector>

class ImageItem : public QQuickPaintedItem
{
	Q_OBJECT
	Q_DISABLE_COPY(ImageItem)

    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(QVector2D dims READ dims NOTIFY dimsChanged)

public:
    ImageItem(QQuickItem *parent = nullptr);
    Q_INVOKABLE void setImage(const QImage &image);
    void paint(QPainter *painter);
    const QImage &image() const { return mImage; }
    QVector2D dims() const { return QVector2D(mImage.width(), mImage.height()); }

signals:
    void imageChanged();
    void dimsChanged(const QVector2D dims);

private:
    QImage mImage;
};
#endif // IMAGEITEM_H
