#pragma once

#include <QMainWindow>
#include <QUrl>

class BrowserTab;
class QTabWidget;
class QWebEnginePage;
class QWebEngineProfile;

class BrowserWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);

private slots:
    void addTab();
    void closeTab(int index);
    void updateWindowTitle();

private:
    void addTabWithUrl(const QUrl &url);
    void addTabWithPage(QWebEnginePage *page);
    BrowserTab *currentTab() const;
    BrowserTab *tabAt(int index) const;
    void cycleTabs(int delta);
    void updateTabChrome(BrowserTab *tab);
    void wireTab(BrowserTab *tab);

    QTabWidget *tabs_;
    QWebEngineProfile *profile_;
};
