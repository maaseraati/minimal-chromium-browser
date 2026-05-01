#include "browser_tab.h"

#include "browser_window.h"

#include <QWebEnginePage>

namespace morphine {

BrowserTab::BrowserTab(BrowserWindow *window) : QWebEngineView(static_cast<QWidget *>(window)), m_window(window) {}

QWebEngineView *BrowserTab::createWindow(QWebEnginePage::WebWindowType)
{
    return m_window->addTab(true);
}

}  // namespace morphine
