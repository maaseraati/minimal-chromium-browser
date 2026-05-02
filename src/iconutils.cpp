#include "iconutils.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QSvgRenderer>

namespace IconUtils {

QIcon coloredSvg(const QString &resourcePath, const QColor &color, int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QSvgRenderer renderer(resourcePath);
        renderer.render(&painter);
    }

    const QRgb target = color.rgb();
    for (int y = 0; y < image.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const int a = qAlpha(row[x]);
            if (a == 0) {
                continue;
            }
            row[x] = qRgba(qRed(target) * a / 255,
                           qGreen(target) * a / 255,
                           qBlue(target) * a / 255,
                           a);
        }
    }

    return QIcon(QPixmap::fromImage(image));
}

} // namespace IconUtils
