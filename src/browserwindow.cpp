#include "browserwindow.h"

#include "browsertab.h"
#include "chrometabbar.h"
#include "iconutils.h"
#include "settingsdialog.h"
#include "thememanager.h"

#include <QCloseEvent>
#include <QColor>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEngineCookieStore>
#include <QWebEngineDownloadRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QWindow>

namespace {

class BrowserTabWidget final : public QTabWidget {
public:
    using QTabWidget::setTabBar;
};

} // namespace

BrowserWindow::BrowserWindow(QWidget *parent)
    : BrowserWindow(false, parent)
{
}

BrowserWindow::BrowserWindow(bool isPrivate, QWidget *parent)
    : QMainWindow(parent),
      tabs_(new BrowserTabWidget),
      profile_(isPrivate ? new QWebEngineProfile(this)
                         : new QWebEngineProfile(QStringLiteral("morphine"), this)),
      settings_(QStringLiteral("Morphine"), QStringLiteral("Morphine")),
      bookmarksList_(new QListWidget(this)),
      historyList_(new QListWidget(this)),
      sidePanel_(new QTabWidget(this)),
      findBar_(new QWidget(this)),
      findInput_(new QLineEdit(this)),
      isPrivate_(isPrivate)
{
    if (!isPrivate_) {
        profile_->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
        profile_->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
        profile_->setCachePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
        profile_->setPersistentStoragePath(
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, false);

    tabs_->setDocumentMode(true);
    auto *chromeTabBar = new ChromeTabBar(tabs_);
    static_cast<BrowserTabWidget *>(tabs_)->setTabBar(chromeTabBar);
    tabs_->setMovable(true);
    tabs_->setTabsClosable(true);
    tabs_->setElideMode(Qt::ElideRight);
    tabs_->tabBar()->setExpanding(false);
    tabs_->tabBar()->installEventFilter(this);

    setupDownloads();
    setupFindBar();
    setupSidePanel();

    auto *rightPane = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(findBar_);
    rightLayout->addWidget(tabs_, 1);

    auto *splitter = new QSplitter(this);
    splitter->addWidget(sidePanel_);
    splitter->addWidget(rightPane);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({0, 1280});
    setCentralWidget(splitter);

    newTabButton_ = new QToolButton(this);
    newTabButton_->setToolTip(QStringLiteral("New tab"));
    newTabButton_->setAutoRaise(true);
    newTabButton_->setCursor(Qt::PointingHandCursor);
    newTabButton_->setIconSize(QSize(17, 17));

    minButton_ = new QToolButton(this);
    minButton_->setToolTip(QStringLiteral("Minimize"));
    minButton_->setAutoRaise(true);
    minButton_->setCursor(Qt::PointingHandCursor);
    minButton_->setIconSize(QSize(16, 16));

    maxButton_ = new QToolButton(this);
    maxButton_->setToolTip(QStringLiteral("Maximize"));
    maxButton_->setAutoRaise(true);
    maxButton_->setCursor(Qt::PointingHandCursor);
    maxButton_->setIconSize(QSize(16, 16));

    closeButton_ = new QToolButton(this);
    closeButton_->setObjectName(QStringLiteral("windowClose"));
    closeButton_->setToolTip(QStringLiteral("Close"));
    closeButton_->setAutoRaise(true);
    closeButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setIconSize(QSize(16, 16));

    auto *rightCorner = new QWidget(this);
    auto *rightLayoutCorner = new QHBoxLayout(rightCorner);
    rightLayoutCorner->setContentsMargins(4, 0, 8, 0);
    rightLayoutCorner->setSpacing(2);
    rightLayoutCorner->addWidget(newTabButton_);
    rightLayoutCorner->addSpacing(8);
    rightLayoutCorner->addWidget(minButton_);
    rightLayoutCorner->addWidget(maxButton_);
    rightLayoutCorner->addWidget(closeButton_);
    tabs_->setCornerWidget(rightCorner, Qt::TopRightCorner);

    connect(minButton_, &QToolButton::clicked, this, &BrowserWindow::showMinimized);
    connect(maxButton_, &QToolButton::clicked, this, [this] {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeButton_, &QToolButton::clicked, this, &BrowserWindow::close);

    menuButton_ = new QToolButton(this);
    menuButton_->setToolTip(QStringLiteral("Menu"));
    menuButton_->setAutoRaise(true);
    menuButton_->setCursor(Qt::PointingHandCursor);
    menuButton_->setIconSize(QSize(18, 18));
    menuButton_->setPopupMode(QToolButton::InstantPopup);
    auto *menu = new QMenu(menuButton_);
    auto *newPrivateAction = menu->addAction(QStringLiteral("New private window"));
    newPrivateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    connect(newPrivateAction, &QAction::triggered, this, &BrowserWindow::openPrivateWindow);
    menu->addSeparator();
    auto *toggleThemeAction = menu->addAction(QStringLiteral("Toggle light theme"));
    toggleThemeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));
    connect(toggleThemeAction, &QAction::triggered, ThemeManager::instance(), &ThemeManager::toggle);
    menu->addSeparator();
    auto *settingsAction = menu->addAction(QStringLiteral("Settings\u2026"));
    settingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    connect(settingsAction, &QAction::triggered, this, &BrowserWindow::showSettings);
    menu->addSeparator();
    auto *bookmarksAction = menu->addAction(QStringLiteral("Bookmarks"));
    bookmarksAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(bookmarksAction, &QAction::triggered, this, [this] { showSidePanel(0); });
    auto *historyAction = menu->addAction(QStringLiteral("History"));
    historyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    connect(historyAction, &QAction::triggered, this, [this] { showSidePanel(1); });
    if (isPrivate_) {
        bookmarksAction->setVisible(false);
        historyAction->setVisible(false);
    }
    menuButton_->setMenu(menu);
    auto *leftCorner = new QWidget(this);
    auto *leftLayoutCorner = new QHBoxLayout(leftCorner);
    leftLayoutCorner->setContentsMargins(8, 0, 4, 0);
    leftLayoutCorner->setSpacing(2);
    leftLayoutCorner->addWidget(menuButton_);
    tabs_->setCornerWidget(leftCorner, Qt::TopLeftCorner);

    refreshChromeIcons();
    updateMaximizeIcon();
    connect(ThemeManager::instance(), &ThemeManager::lightChanged, this,
            [this](bool) { refreshChromeIcons(); });

    connect(newTabButton_, &QToolButton::clicked, this, &BrowserWindow::addTab);
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), this,
                  [] { ThemeManager::instance()->toggle(); });
    connect(tabs_, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(tabs_, &QTabWidget::currentChanged, this, [this, chromeTabBar] {
        chromeTabBar->animateSelectionToCurrent();
        updateWindowTitle();
    });
    connect(ThemeManager::instance(), &ThemeManager::lightChanged, chromeTabBar,
            &ChromeTabBar::refreshTheme);

    new QShortcut(QKeySequence::AddTab, this, SLOT(addTab()));
    new QShortcut(QKeySequence::Close, this, [this] {
        closeTab(tabs_->currentIndex());
    });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this, [this] {
        if (auto *tab = currentTab()) {
            tab->focusAddressBar();
        }
    });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), this, [this] {
        cycleTabs(1);
    });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab), this, [this] {
        cycleTabs(-1);
    });
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Left), this, [this] {
        if (auto *tab = currentTab()) {
            tab->view()->back();
        }
    });
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Right), this, [this] {
        if (auto *tab = currentTab()) {
            tab->view()->forward();
        }
    });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, [this] {
        if (auto *tab = currentTab()) {
            tab->view()->reload();
        }
    });
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Home), this, [this] {
        if (auto *tab = currentTab()) {
            tab->loadHome();
        }
    });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this, SLOT(addBookmark()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this, [this] {
        showSidePanel(0);
    });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this, [this] {
        showSidePanel(1);
    });
    new QShortcut(QKeySequence::Find, this, SLOT(showFindBar()));
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hideFindBar()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma), this, SLOT(showSettings()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N), this, SLOT(openPrivateWindow()));

    resize(1280, 820);
    if (isPrivate_) {
        setStyleSheet(QStringLiteral(
            "QMainWindow { background: #1a1230; } "
            "QTabWidget::pane { background: #1a1230; } "
            "QTabBar { background: #1a1230; } "
            "QTabWidget::corner { background: #1a1230; } "
            "QStatusBar { background: #2a1a4a; color: #d8c8ff; }"));
        statusBar()->showMessage(QStringLiteral(
            "Private mode \u2014 history, cookies and downloads are not saved."));
    } else {
        loadBookmarks();
        loadHistory();
    }
    restoreSession();
}

bool BrowserWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == tabs_->tabBar()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                const int idx = tabs_->tabBar()->tabAt(me->pos());
                if (idx == -1 && windowHandle()) {
                    windowHandle()->startSystemMove();
                    return true;
                }
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (tabs_->tabBar()->tabAt(me->pos()) == -1) {
                if (isMaximized()) {
                    showNormal();
                } else {
                    showMaximized();
                }
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void BrowserWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeIcon();
    }
}

void BrowserWindow::refreshChromeIcons()
{
    const bool light = ThemeManager::instance()->isLight();
    const QColor iconColor(light ? QStringLiteral("#44474e") : QStringLiteral("#c4c6cf"));
    if (menuButton_) {
        menuButton_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/menu.svg"), iconColor));
    }
    if (newTabButton_) {
        newTabButton_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/plus.svg"), iconColor));
    }
    if (minButton_) {
        minButton_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/window-min.svg"), iconColor));
    }
    if (closeButton_) {
        closeButton_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/window-close.svg"), iconColor));
    }
    updateMaximizeIcon();
}

void BrowserWindow::updateMaximizeIcon()
{
    if (!maxButton_) {
        return;
    }
    const bool light = ThemeManager::instance()->isLight();
    const QColor iconColor(light ? QStringLiteral("#44474e") : QStringLiteral("#c4c6cf"));
    const QString resource = isMaximized()
        ? QStringLiteral(":/assets/window-restore.svg")
        : QStringLiteral(":/assets/window-max.svg");
    maxButton_->setIcon(IconUtils::coloredSvg(resource, iconColor));
    maxButton_->setToolTip(isMaximized() ? QStringLiteral("Restore") : QStringLiteral("Maximize"));
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if (!isPrivate_) {
        saveSession();
    }
    QMainWindow::closeEvent(event);
}

void BrowserWindow::openPrivateWindow()
{
    auto *window = new BrowserWindow(true);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void BrowserWindow::addBookmark()
{
    if (isPrivate_) {
        statusBar()->showMessage(QStringLiteral("Bookmarks are disabled in private mode"), 3000);
        return;
    }

    auto *tab = currentTab();
    if (!tab || !tab->isRestorableUrl()) {
        return;
    }

    const QString url = tab->url().toString();
    for (int i = 0; i < bookmarksList_->count(); ++i) {
        if (bookmarksList_->item(i)->data(Qt::UserRole).toString() == url) {
            statusBar()->showMessage(QStringLiteral("Already bookmarked"), 3000);
            return;
        }
    }

    auto *item = new QListWidgetItem(tab->icon(), tab->title(), bookmarksList_);
    item->setData(Qt::UserRole, url);
    item->setToolTip(url);
    saveBookmarks();
    statusBar()->showMessage(QStringLiteral("Bookmark added"), 3000);
}

void BrowserWindow::addTab()
{
    addTabWithUrl(QUrl());
}

void BrowserWindow::addTabWithUrl(const QUrl &url)
{
    auto *tab = new BrowserTab(profile_, nullptr, this);
    wireTab(tab);

    const int index = tabs_->addTab(tab, tab->title());
    tabs_->setCurrentIndex(index);
    refreshTabMetrics();

    if (!url.isEmpty()) {
        tab->view()->load(url);
    }

    tab->focusAddressBar();
    if (!isPrivate_) {
        saveSession();
    }
}

void BrowserWindow::addTabWithPage(QWebEnginePage *page)
{
    auto *tab = new BrowserTab(profile_, page, this);
    wireTab(tab);

    const int index = tabs_->addTab(tab, tab->icon(), tab->title());
    tabs_->setCurrentIndex(index);
    refreshTabMetrics();
    if (!isPrivate_) {
        saveSession();
    }
}

void BrowserWindow::closeTab(int index)
{
    if (tabs_->count() == 1) {
        close();
        return;
    }

    QWidget *widget = tabs_->widget(index);
    tabs_->removeTab(index);
    widget->deleteLater();
    refreshTabMetrics();
    updateWindowTitle();
    if (!isPrivate_) {
        saveSession();
    }
}

void BrowserWindow::hideFindBar()
{
    findBar_->hide();
    if (auto *tab = currentTab()) {
        tab->findInPage(QString());
        tab->view()->setFocus();
    }
}

void BrowserWindow::openSelectedListItem(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    const QUrl url(item->data(Qt::UserRole).toString());
    if (url.isValid()) {
        addTabWithUrl(url);
    }
}

void BrowserWindow::showFindBar()
{
    findBar_->show();
    findInput_->setFocus();
    findInput_->selectAll();
}

void BrowserWindow::updateWindowTitle()
{
    auto *tab = currentTab();
    const QString title = tab ? tab->title() : QStringLiteral("Morphine");
    const QString prefix = tab && tab->isLoading() ? QStringLiteral("Loading ") : QString();
    setWindowTitle(QStringLiteral("%1%2 — Morphine").arg(prefix, title));
}

void BrowserWindow::addHistoryEntry(const QString &title, const QUrl &url)
{
    if (isPrivate_ || !url.isValid() || url.scheme() == QStringLiteral("morphine") || title.isEmpty()) {
        return;
    }

    const QString urlText = url.toString();
    for (int i = 0; i < historyList_->count(); ++i) {
        if (historyList_->item(i)->data(Qt::UserRole).toString() == urlText) {
            delete historyList_->takeItem(i);
            break;
        }
    }

    auto *item = new QListWidgetItem(title, historyList_);
    item->setData(Qt::UserRole, urlText);
    item->setToolTip(urlText);
    historyList_->insertItem(0, item);

    while (historyList_->count() > 100) {
        delete historyList_->takeItem(historyList_->count() - 1);
    }

    QStringList entries;
    entries.reserve(historyList_->count());
    for (int i = 0; i < historyList_->count(); ++i) {
        const auto *historyItem = historyList_->item(i);
        entries << QStringLiteral("%1\t%2").arg(
            historyItem->text(), historyItem->data(Qt::UserRole).toString());
    }
    settings_.setValue(QStringLiteral("history"), entries);
}

BrowserTab *BrowserWindow::currentTab() const
{
    return tabAt(tabs_->currentIndex());
}

BrowserTab *BrowserWindow::tabAt(int index) const
{
    return qobject_cast<BrowserTab *>(tabs_->widget(index));
}

void BrowserWindow::cycleTabs(int delta)
{
    const int count = tabs_->count();
    if (count < 2) {
        return;
    }

    const int nextIndex = (tabs_->currentIndex() + delta + count) % count;
    tabs_->setCurrentIndex(nextIndex);
}

void BrowserWindow::loadBookmarks()
{
    const QStringList entries = settings_.value(QStringLiteral("bookmarks")).toStringList();
    for (const QString &entry : entries) {
        const QStringList parts = entry.split('\t');
        if (parts.size() != 2) {
            continue;
        }

        auto *item = new QListWidgetItem(parts.at(0), bookmarksList_);
        item->setData(Qt::UserRole, parts.at(1));
        item->setToolTip(parts.at(1));
    }
}

void BrowserWindow::loadHistory()
{
    const QStringList entries = settings_.value(QStringLiteral("history")).toStringList();
    for (const QString &entry : entries) {
        const QStringList parts = entry.split('\t');
        if (parts.size() != 2) {
            continue;
        }

        auto *item = new QListWidgetItem(parts.at(0), historyList_);
        item->setData(Qt::UserRole, parts.at(1));
        item->setToolTip(parts.at(1));
    }
}

void BrowserWindow::restoreSession()
{
    if (!isPrivate_ && SettingsDialog::restoreSessionEnabled()) {
        const QStringList urls = settings_.value(QStringLiteral("session/urls")).toStringList();
        for (const QString &url : urls) {
            const QUrl parsedUrl(url);
            if (parsedUrl.isValid()) {
                addTabWithUrl(parsedUrl);
            }
        }
    }

    if (tabs_->count() == 0) {
        addTab();
    }

    if (!isPrivate_) {
        const int index = settings_.value(QStringLiteral("session/currentIndex"), 0).toInt();
        if (index >= 0 && index < tabs_->count()) {
            tabs_->setCurrentIndex(index);
        }
    }
}

void BrowserWindow::saveBookmarks()
{
    QStringList entries;
    entries.reserve(bookmarksList_->count());
    for (int i = 0; i < bookmarksList_->count(); ++i) {
        const auto *item = bookmarksList_->item(i);
        entries << QStringLiteral("%1\t%2").arg(item->text(), item->data(Qt::UserRole).toString());
    }
    settings_.setValue(QStringLiteral("bookmarks"), entries);
}

void BrowserWindow::saveSession()
{
    QStringList urls;
    for (int i = 0; i < tabs_->count(); ++i) {
        const auto *tab = tabAt(i);
        if (tab && tab->isRestorableUrl()) {
            urls << tab->url().toString();
        }
    }

    settings_.setValue(QStringLiteral("session/urls"), urls);
    settings_.setValue(QStringLiteral("session/currentIndex"), tabs_->currentIndex());
}

void BrowserWindow::setupDownloads()
{
    applyDownloadPath();

    connect(profile_, &QWebEngineProfile::downloadRequested, this, [this](QWebEngineDownloadRequest *download) {
        const QString directory =
            profile_->downloadPath().isEmpty() ? QDir::homePath() : profile_->downloadPath();
        download->setDownloadDirectory(directory);
        download->setDownloadFileName(download->suggestedFileName());
        download->accept();

        const QString fileName = QFileInfo(download->downloadFileName()).fileName();
        statusBar()->showMessage(QStringLiteral("Downloading %1").arg(fileName), 5000);
        connect(download, &QWebEngineDownloadRequest::isFinishedChanged, this, [this, download, fileName] {
            const QString message = download->state() == QWebEngineDownloadRequest::DownloadCompleted
                ? QStringLiteral("Downloaded %1").arg(fileName)
                : QStringLiteral("Download failed: %1").arg(fileName);
            statusBar()->showMessage(message, 7000);
        });
    });
}

void BrowserWindow::applyDownloadPath()
{
    const QString configured = SettingsDialog::downloadDirectory();
    if (!configured.isEmpty()) {
        QDir().mkpath(configured);
        profile_->setDownloadPath(configured);
    }
}

void BrowserWindow::showSettings()
{
    SettingsDialog dialog(this);
    connect(&dialog, &SettingsDialog::settingsChanged, this, [this] {
        applyDownloadPath();
    });
    connect(&dialog, &SettingsDialog::clearHistoryRequested, this, &BrowserWindow::clearHistoryAll);
    connect(&dialog, &SettingsDialog::clearBookmarksRequested, this, &BrowserWindow::clearBookmarksAll);
    connect(&dialog, &SettingsDialog::clearBrowsingDataRequested, this, &BrowserWindow::clearBrowsingData);
    dialog.exec();
}

void BrowserWindow::clearHistoryAll()
{
    historyList_->clear();
    settings_.remove(QStringLiteral("history"));
    statusBar()->showMessage(QStringLiteral("History cleared"), 3000);
}

void BrowserWindow::clearBookmarksAll()
{
    bookmarksList_->clear();
    saveBookmarks();
    statusBar()->showMessage(QStringLiteral("Bookmarks cleared"), 3000);
}

void BrowserWindow::clearBrowsingData()
{
    profile_->cookieStore()->deleteAllCookies();
    profile_->clearAllVisitedLinks();
    profile_->clearHttpCache();
    statusBar()->showMessage(QStringLiteral("Cookies and cache cleared"), 3000);
}

void BrowserWindow::setupFindBar()
{
    auto *label = new QLabel(QStringLiteral("Find"), findBar_);
    auto *previousButton = new QPushButton(QStringLiteral("Previous"), findBar_);
    auto *nextButton = new QPushButton(QStringLiteral("Next"), findBar_);
    auto *closeButton = new QPushButton(QStringLiteral("Close"), findBar_);

    auto *layout = new QHBoxLayout(findBar_);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->addWidget(label);
    layout->addWidget(findInput_, 1);
    layout->addWidget(previousButton);
    layout->addWidget(nextButton);
    layout->addWidget(closeButton);

    findBar_->hide();

    connect(findInput_, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (auto *tab = currentTab()) {
            tab->findInPage(text);
        }
    });
    connect(nextButton, &QPushButton::clicked, this, [this] {
        if (auto *tab = currentTab()) {
            tab->findInPage(findInput_->text());
        }
    });
    connect(previousButton, &QPushButton::clicked, this, [this] {
        if (auto *tab = currentTab()) {
            tab->findInPage(findInput_->text(), true);
        }
    });
    connect(closeButton, &QPushButton::clicked, this, &BrowserWindow::hideFindBar);
}

void BrowserWindow::setupSidePanel()
{
    sidePanel_->setMaximumWidth(280);
    sidePanel_->addTab(bookmarksList_, QStringLiteral("Bookmarks"));
    sidePanel_->addTab(historyList_, QStringLiteral("History"));
    sidePanel_->hide();

    connect(bookmarksList_, &QListWidget::itemDoubleClicked, this, &BrowserWindow::openSelectedListItem);
    connect(historyList_, &QListWidget::itemDoubleClicked, this, &BrowserWindow::openSelectedListItem);
}

void BrowserWindow::showSidePanel(int pageIndex)
{
    if (sidePanel_->isVisible() && sidePanel_->currentIndex() == pageIndex) {
        sidePanel_->hide();
        return;
    }

    sidePanel_->setCurrentIndex(pageIndex);
    sidePanel_->show();
}

void BrowserWindow::refreshTabMetrics()
{
    tabs_->tabBar()->setFixedHeight(40);
    tabs_->tabBar()->setUsesScrollButtons(true);
    tabs_->tabBar()->setIconSize(QSize(16, 16));
}

void BrowserWindow::updateTabChrome(BrowserTab *tab)
{
    const int index = tabs_->indexOf(tab);
    if (index < 0) {
        return;
    }

    const QString title = tab->title().left(32);
    tabs_->setTabText(index, tab->isLoading() ? QStringLiteral("◌ %1").arg(title) : title);
    tabs_->setTabToolTip(index, tab->url().isEmpty() ? tab->title() : tab->url().toString());
    tabs_->setTabIcon(index, tab->icon());
    updateWindowTitle();
    if (!isPrivate_) {
        saveSession();
    }
}

void BrowserWindow::wireTab(BrowserTab *tab)
{
    connect(tab, &BrowserTab::titleChanged, this, [this, tab](const QString &title) {
        Q_UNUSED(title);
        updateTabChrome(tab);
        addHistoryEntry(tab->title(), tab->url());
    });
    connect(tab, &BrowserTab::iconChanged, this, [this, tab] {
        updateTabChrome(tab);
    });
    connect(tab, &BrowserTab::loadingChanged, this, [this, tab] {
        updateTabChrome(tab);
    });
    connect(tab, &BrowserTab::urlChanged, this, [this, tab](const QUrl &url) {
        Q_UNUSED(url);
        updateTabChrome(tab);
        addHistoryEntry(tab->title(), tab->url());
    });
    connect(tab, &BrowserTab::newWindowPageRequested, this, [this](QWebEnginePage *page) {
        addTabWithPage(page);
    });
    connect(tab, &BrowserTab::closeRequested, this, [this, tab] {
        closeTab(tabs_->indexOf(tab));
    });
}
