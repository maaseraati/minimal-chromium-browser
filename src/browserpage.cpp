#include "browserpage.h"

#include <QWebEngineProfile>

BrowserPage::BrowserPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
}

QWebEnginePage *BrowserPage::createWindow(WebWindowType type)
{
    Q_UNUSED(type);

    auto *page = new BrowserPage(profile());
    emit newWindowPageCreated(page);
    return page;
}
