#include "browserwindow.h"

#include "browsertab.h"

#include <QKeySequence>
#include <QShortcut>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
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

    connect(newTabButton, &QToolButton::clicked, this, [this] {
        addTab();
    });
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

    resize(1280, 820);
    addTab();
}

void BrowserWindow::addTab(const QUrl &url)
{
    auto *tab = new BrowserTab(profile_, this);
    wireTab(tab);

    const int index = tabs_->addTab(tab, tab->title());
    tabs_->setCurrentIndex(index);

    if (!url.isEmpty()) {
        tab->view()->load(url);
    }

    tab->focusAddressBar();
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
    const QString title = currentTab() ? currentTab()->title() : QStringLiteral("Morphine");
    setWindowTitle(QStringLiteral("%1 — Morphine").arg(title));
}

BrowserTab *BrowserWindow::currentTab() const
{
    return tabAt(tabs_->currentIndex());
}

BrowserTab *BrowserWindow::tabAt(int index) const
{
    return qobject_cast<BrowserTab *>(tabs_->widget(index));
}

void BrowserWindow::wireTab(BrowserTab *tab)
{
    connect(tab, &BrowserTab::titleChanged, this, [this, tab](const QString &title) {
        const int index = tabs_->indexOf(tab);
        if (index >= 0) {
            tabs_->setTabText(index, title.left(32));
            tabs_->setTabToolTip(index, title);
        }
        updateWindowTitle();
    });
    connect(tab, &BrowserTab::urlChanged, this, [this, tab](const QUrl &url) {
        const int index = tabs_->indexOf(tab);
        if (index >= 0) {
            tabs_->setTabToolTip(index, url.toString());
        }
    });
    connect(tab, &BrowserTab::closeRequested, this, [this, tab] {
        closeTab(tabs_->indexOf(tab));
    });
}
