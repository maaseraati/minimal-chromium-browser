#pragma once

#include <QIcon>

class QColor;
class QString;

namespace IconUtils {

QIcon coloredSvg(const QString &resourcePath, const QColor &color, int size = 24);

} // namespace IconUtils
