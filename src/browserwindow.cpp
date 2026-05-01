#include "browserwindow.h"

#include "browsertab.h"

#include <QKeySequence>
#include <QShortcut>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent),
      tabs_(new QTabWidget(this)),
      profile_(new QWebEngineProfile(QStringLiteral("morphine"), this))
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
    setCentralWidget(tabs_);

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

    resize(1280, 820);
    addTab();
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
}

void BrowserWindow::addTabWithPage(QWebEnginePage *page)
{
    auto *tab = new BrowserTab(profile_, page, this);
    wireTab(tab);

    const int index = tabs_->addTab(tab, tab->icon(), tab->title());
    tabs_->setCurrentIndex(index);
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
}

void BrowserWindow::updateWindowTitle()
{
    auto *tab = currentTab();
    const QString title = tab ? tab->title() : QStringLiteral("Morphine");
    const QString prefix = tab && tab->isLoading() ? QStringLiteral("Loading ") : QString();
    setWindowTitle(QStringLiteral("%1%2 — Morphine").arg(prefix, title));
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
}

void BrowserWindow::wireTab(BrowserTab *tab)
{
    connect(tab, &BrowserTab::titleChanged, this, [this, tab](const QString &title) {
        Q_UNUSED(title);
        updateTabChrome(tab);
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
    });
    connect(tab, &BrowserTab::newWindowPageRequested, this, [this](QWebEnginePage *page) {
        addTabWithPage(page);
    });
    connect(tab, &BrowserTab::closeRequested, this, [this, tab] {
        closeTab(tabs_->indexOf(tab));
    });
}
