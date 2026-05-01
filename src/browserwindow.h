#pragma once

#include <QMainWindow>
#include <QUrl>

class BrowserTab;
class QTabWidget;
class QWebEngineProfile;

class BrowserWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);

private slots:
    void addTab(const QUrl &url = QUrl());
    void closeTab(int index);
    void updateWindowTitle();

private:
    BrowserTab *currentTab() const;
    BrowserTab *tabAt(int index) const;
    void wireTab(BrowserTab *tab);

    QTabWidget *tabs_;
    QWebEngineProfile *profile_;
};
