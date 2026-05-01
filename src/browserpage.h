#pragma once

#include <QWebEnginePage>

class BrowserPage final : public QWebEnginePage {
    Q_OBJECT

public:
    explicit BrowserPage(QWebEngineProfile *profile, QObject *parent = nullptr);

signals:
    void newWindowPageCreated(QWebEnginePage *page);

protected:
    QWebEnginePage *createWindow(WebWindowType type) override;
};
