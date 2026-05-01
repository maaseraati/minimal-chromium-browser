#pragma once

#include <QWebEngineView>

namespace morphine {

class BrowserWindow;

class BrowserTab : public QWebEngineView {
public:
    explicit BrowserTab(BrowserWindow *window);

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    BrowserWindow *m_window;
};

}  // namespace morphine
