#include "thememanager.h"

#include <QSettings>

ThemeManager *ThemeManager::instance()
{
    static ThemeManager s;
    return &s;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), light_(false)
{
    QSettings settings(QStringLiteral("Morphine"), QStringLiteral("Morphine"));
    light_ = settings.value(QStringLiteral("theme/light"), false).toBool();
}

void ThemeManager::setLight(bool light)
{
    if (light_ == light) {
        return;
    }
    light_ = light;
    QSettings settings(QStringLiteral("Morphine"), QStringLiteral("Morphine"));
    settings.setValue(QStringLiteral("theme/light"), light);
    emit lightChanged(light);
}

void ThemeManager::toggle()
{
    setLight(!light_);
}
