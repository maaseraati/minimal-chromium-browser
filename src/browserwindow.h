#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QUrl>

class BrowserTab;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QTabWidget;
class QToolButton;
class QWebEnginePage;
class QWebEngineProfile;

class BrowserWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    explicit BrowserWindow(bool isPrivate, QWidget *parent = nullptr);

    bool isPrivate() const { return isPrivate_; }

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void addTab();
    void addBookmark();
    void closeTab(int index);
    void hideFindBar();
    void openPrivateWindow();
    void openSelectedListItem(QListWidgetItem *item);
    void showFindBar();
    void showSettings();
    void updateWindowTitle();

private:
    void addHistoryEntry(const QString &title, const QUrl &url);
    void addTabWithUrl(const QUrl &url);
    void addTabWithPage(QWebEnginePage *page);
    void applyDownloadPath();
    void clearBookmarksAll();
    void clearBrowsingData();
    void clearHistoryAll();
    BrowserTab *currentTab() const;
    BrowserTab *tabAt(int index) const;
    void cycleTabs(int delta);
    void loadBookmarks();
    void loadHistory();
    void restoreSession();
    void saveBookmarks();
    void saveSession();
    void setupDownloads();
    void setupFindBar();
    void setupSidePanel();
    void showSidePanel(int pageIndex);
    void updateTabChrome(BrowserTab *tab);
    void wireTab(BrowserTab *tab);
    void refreshChromeIcons();
    void updateMaximizeIcon();

    QTabWidget *tabs_;
    QWebEngineProfile *profile_;
    QSettings settings_;
    QListWidget *bookmarksList_;
    QListWidget *historyList_;
    QTabWidget *sidePanel_;
    QWidget *findBar_;
    QLineEdit *findInput_;
    QToolButton *menuButton_;
    QToolButton *newTabButton_;
    QToolButton *minButton_;
    QToolButton *maxButton_;
    QToolButton *closeButton_;
    bool isPrivate_;
};
