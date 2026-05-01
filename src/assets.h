#pragma once

#include <QIcon>
#include <QString>

namespace morphine {

constexpr auto AppTitle = "morphine";
constexpr auto HomeUrl = "morphine://home";
constexpr auto HistoryUrl = "morphine://history";
constexpr auto GoogleSearchUrl = "https://www.google.com/search?q=";
constexpr auto LogoFileName = "morphine_logo.png";

constexpr auto PrimaryColor = "#2f7eea";
constexpr auto OnSurface = "#1e3558";
constexpr auto OnSurfaceVariant = "#476285";

struct HistoryEntry {
    QString title;
    QString url;
};

QIcon svgIcon(const QString &name, const QString &color = PrimaryColor, int size = 24);
QString htmlEscaped(const QString &value);
QString homeHtml();
bool isSearchQuery(const QString &text);

}  // namespace morphine
