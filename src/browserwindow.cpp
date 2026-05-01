#include "browserwindow.h"

#include "browsertab.h"

#include <QCloseEvent>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
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
#include <QWebEngineDownloadRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent),
      tabs_(new QTabWidget(this)),
      profile_(new QWebEngineProfile(QStringLiteral("morphine"), this)),
      settings_(QStringLiteral("Morphine"), QStringLiteral("Morphine")),
      bookmarksList_(new QListWidget(this)),
      historyList_(new QListWidget(this)),
      sidePanel_(new QTabWidget(this)),
      findBar_(new QWidget(this)),
      findInput_(new QLineEdit(this))
{
    profile_->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile_->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    profile_->setCachePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    profile_->setPersistentStoragePath(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    tabs_->setDocumentMode(true);
    tabs_->setMovable(true);
    tabs_->setTabsClosable(true);
    tabs_->setElideMode(Qt::ElideRight);
    tabs_->tabBar()->setExpanding(false);

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

    auto *newTabButton = new QToolButton(this);
    newTabButton->setText("+");
    newTabButton->setToolTip("New tab");
    newTabButton->setAutoRaise(true);
    newTabButton->setCursor(Qt::PointingHandCursor);
    tabs_->setCornerWidget(newTabButton, Qt::TopRightCorner);

    connect(newTabButton, &QToolButton::clicked, this, &BrowserWindow::addTab);
    connect(tabs_, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(tabs_, &QTabWidget::currentChanged, this, &BrowserWindow::updateWindowTitle);

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

    resize(1280, 820);
    loadBookmarks();
    loadHistory();
    restoreSession();
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    saveSession();
    QMainWindow::closeEvent(event);
}

void BrowserWindow::addBookmark()
{
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

    if (!url.isEmpty()) {
        tab->view()->load(url);
    }

    tab->focusAddressBar();
    saveSession();
}

void BrowserWindow::addTabWithPage(QWebEnginePage *page)
{
    auto *tab = new BrowserTab(profile_, page, this);
    wireTab(tab);

    const int index = tabs_->addTab(tab, tab->icon(), tab->title());
    tabs_->setCurrentIndex(index);
    saveSession();
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
    updateWindowTitle();
    saveSession();
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
    if (!url.isValid() || url.scheme() == QStringLiteral("morphine") || title.isEmpty()) {
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
    const QStringList urls = settings_.value(QStringLiteral("session/urls")).toStringList();
    for (const QString &url : urls) {
        const QUrl parsedUrl(url);
        if (parsedUrl.isValid()) {
            addTabWithUrl(parsedUrl);
        }
    }

    if (tabs_->count() == 0) {
        addTab();
    }

    const int index = settings_.value(QStringLiteral("session/currentIndex"), 0).toInt();
    if (index >= 0 && index < tabs_->count()) {
        tabs_->setCurrentIndex(index);
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
    const QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (!downloadPath.isEmpty()) {
        profile_->setDownloadPath(downloadPath);
    }

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
    saveSession();
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
