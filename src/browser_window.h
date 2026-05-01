#pragma once

#include "assets.h"

#include <QHBoxLayout>
#include <QMainWindow>
#include <QPoint>
#include <QRect>
#include <QStringList>

class QFrame;
class QLineEdit;
class QResizeEvent;
class QStackedWidget;
class QToolButton;

namespace morphine {

class BrowserTab;

class BrowserWindow : public QMainWindow {
public:
    BrowserWindow();

    BrowserTab *addTab(bool switchTo = false);
    void closeTab(int index);
    void selectTab(int index);
    void toggleWindowMaximized();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void buildChrome();
    QFrame *buildAddressPill();
    void buildCentral();
    void installShortcuts();
    void openAddress();
    void loadUrl(const QString &rawUrl);
    void loadHome(BrowserTab *view = nullptr);
    void showHistory();
    QString historyHtml() const;
    BrowserTab *activeWebView();
    void syncTabButtons();
    void updateAddressBar(BrowserTab *view, const QUrl &url);
    void updateTabTitle(BrowserTab *view, const QString &title);
    void updateTabIcon(BrowserTab *view, const QIcon &icon);
    void recordHistory(BrowserTab *view, const QString &title);
    void rememberClosedTab(QWidget *view);
    void reopenLastClosedTab();
    void closeCurrentTab();
    void cycleTab(int delta);
    void focusAddressBar();
    void hardReload();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void toggleFullscreen();
    void minimizeWindow();
    int hitTestEdge(const QPoint &globalPos) const;
    Qt::CursorShape cursorForEdge(int mask) const;
    void updateResizeCursor(int mask);

    QToolButton *m_backButton = nullptr;
    QToolButton *m_forwardButton = nullptr;
    QToolButton *m_reloadButton = nullptr;
    QToolButton *m_historyButton = nullptr;
    QToolButton *m_starButton = nullptr;
    QLineEdit *m_addressBar = nullptr;
    QWidget *m_chromeBar = nullptr;
    QWidget *m_tabButtonsContainer = nullptr;
    QHBoxLayout *m_tabButtonsLayout = nullptr;
    QStackedWidget *m_pages = nullptr;
    QList<HistoryEntry> m_history;
    QStringList m_closedTabs;
    bool m_recordingHistory = true;
    int m_currentTabIndex = -1;
    int m_currentResizeEdge = 0;
    bool m_hasResizeCursor = false;
    bool m_isResizing = false;
    QPoint m_resizeStartGlobal;
    QRect m_resizeStartGeometry;
    static constexpr int ResizeMargin = 6;
};

}  // namespace morphine
