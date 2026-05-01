#include "browser_window.h"

#include "browser_tab.h"
#include "tab_button.h"
#include "tab_strip.h"

#include <QApplication>
#include <QByteArray>
#include <QCloseEvent>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMouseEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWidget>

#include <algorithm>
#include <functional>

namespace morphine {

BrowserWindow::BrowserWindow()
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowTitle(AppTitle);
    resize(1200, 800);
    setMouseTracking(true);
    setStyleSheet(QStringLiteral(R"(
QMainWindow, QWidget#chromeRoot, QWidget#appShell, QStackedWidget#pages { background: #f7fbff; }
QWidget#tabStrip { background: #f7fbff; border-top: 1px solid #d9e8fb; border-left: 1px solid #d9e8fb; border-right: 1px solid #d9e8fb; border-top-left-radius: 18px; border-top-right-radius: 18px; }
QWidget#tabButtonsContainer { max-width: 540px; }
QWidget#chromeBar { background: #edf6ff; border: 1px solid rgba(150,184,236,.36); border-top: 0; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; }
QWidget#browserTab { background: #e8f2ff; border: 1px solid rgba(127,169,232,.32); border-bottom: 0; border-top-left-radius: 15px; border-top-right-radius: 15px; color: #476285; }
QWidget#browserTab:hover { background: #f1f7ff; border-color: rgba(47,126,234,.28); }
QWidget#browserTab[active="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #73abff, stop:1 #4e8ff0); border-top-left-radius: 15px; border-top-right-radius: 15px; color: #fff; }
QLabel#tabBadge { color: #476285; font-size: 14px; font-weight: 700; }
QLabel#tabTitle { color: #243d60; font-size: 15px; font-weight: 600; }
QWidget#browserTab[active="true"] QLabel#tabTitle { color: #fff; }
QWidget#browserTab[active="true"] QLabel#tabBadge { color: #fff; }
QToolButton#tabCloseBtn { background: transparent; border: 0; border-radius: 10px; min-width: 20px; min-height: 20px; max-width: 20px; max-height: 20px; }
QToolButton#tabCloseBtn:hover { background: rgba(255,255,255,.28); }
QToolButton#newTabBtn { background: rgba(47,126,234,.08); border: 0; border-radius: 19px; min-width: 38px; min-height: 38px; max-width: 38px; max-height: 38px; }
QToolButton#newTabBtn:hover { background: rgba(47,126,234,.14); }
QToolButton[chromeNav="true"], QToolButton#dotsBtn { background: transparent; border: 0; border-radius: 20px; min-width: 40px; min-height: 40px; max-height: 40px; }
QToolButton[chromeNav="true"]:hover, QToolButton#dotsBtn:hover { background: rgba(47,126,234,.10); }
QFrame#addressPill { background: #fff; border: 1px solid rgba(151,185,238,.24); border-radius: 25px; }
QFrame#addressPill:hover { border-color: rgba(47,126,234,.35); }
QFrame#addressPill QLineEdit { border: 0; background: transparent; color: #1e3558; font-size: 20px; selection-background-color: #cfe3ff; selection-color: #12315b; }
QFrame#addressPill QToolButton { background: transparent; border: 0; border-radius: 14px; min-width: 28px; min-height: 28px; }
QFrame#addressPill QToolButton:hover { background: rgba(47,126,234,.10); }
QToolButton#avatar { background: #3b82ee; color: #fff; border: 0; border-radius: 22px; font-weight: 800; font-size: 18px; min-width: 44px; min-height: 44px; max-width: 44px; max-height: 44px; }
QToolButton#avatar:hover { background: #2f73dc; }
QWidget#windowControls { background: transparent; }
QToolButton[windowControl="true"] { background: transparent; border: 0; border-radius: 15px; min-width: 50px; min-height: 34px; font-size: 25px; color: #243d60; }
QToolButton[windowControl="true"]:hover { background: rgba(47,126,234,.10); }
QToolButton#windowClose:hover { background: #f06a4d; color: white; }
)"));

    buildChrome();
    buildCentral();
    installShortcuts();
    qApp->installEventFilter(this);
    addTab(true);
}

void BrowserWindow::buildChrome()
{
    auto makeNavButton = [](const QString &icon, const QString &tooltip) {
        auto *button = new QToolButton;
        button->setIcon(svgIcon(icon, OnSurface, 25));
        button->setIconSize(QSize(25, 25));
        button->setToolTip(tooltip);
        button->setProperty("chromeNav", true);
        button->setCursor(Qt::PointingHandCursor);
        button->setAutoRaise(true);
        return button;
    };

    m_backButton = makeNavButton("arrow_back", "Back");
    connect(m_backButton, &QToolButton::clicked, this, [this]() { activeWebView()->back(); });
    m_forwardButton = makeNavButton("arrow_forward", "Forward");
    connect(m_forwardButton, &QToolButton::clicked, this, [this]() { activeWebView()->forward(); });
    m_reloadButton = makeNavButton("refresh", "Reload");
    connect(m_reloadButton, &QToolButton::clicked, this, [this]() { activeWebView()->reload(); });
    m_historyButton = makeNavButton("history", "History");
    connect(m_historyButton, &QToolButton::clicked, this, [this]() { showHistory(); });

    auto *addressPill = buildAddressPill();
    auto *dotsButton = new QToolButton;
    dotsButton->setObjectName("dotsBtn");
    dotsButton->setIcon(svgIcon("more_vert", OnSurface, 25));
    dotsButton->setIconSize(QSize(25, 25));
    dotsButton->setCursor(Qt::PointingHandCursor);
    dotsButton->setAutoRaise(true);

    auto *avatarButton = new QToolButton;
    avatarButton->setObjectName("avatar");
    avatarButton->setText("U");
    avatarButton->setCursor(Qt::PointingHandCursor);
    avatarButton->setAutoRaise(true);

    m_chromeBar = new QWidget;
    m_chromeBar->setObjectName("chromeBar");
    m_chromeBar->setFixedHeight(72);
    auto *layout = new QHBoxLayout(m_chromeBar);
    layout->setContentsMargins(28, 11, 22, 11);
    layout->setSpacing(18);
    layout->addWidget(m_backButton);
    layout->addWidget(m_forwardButton);
    layout->addWidget(m_reloadButton);
    layout->addWidget(m_historyButton);
    layout->addSpacing(26);
    layout->addWidget(addressPill, 1);
    layout->addSpacing(28);
    layout->addWidget(dotsButton);
    layout->addWidget(avatarButton);
}

QFrame *BrowserWindow::buildAddressPill()
{
    auto *pill = new QFrame;
    pill->setObjectName("addressPill");
    pill->setFixedHeight(52);
    pill->setMinimumWidth(420);
    pill->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *lockButton = new QToolButton;
    lockButton->setObjectName("lockBtn");
    lockButton->setIcon(svgIcon("lock", "#8a98aa", 17));
    lockButton->setIconSize(QSize(19, 19));
    lockButton->setEnabled(false);
    lockButton->setAutoRaise(true);

    m_addressBar = new QLineEdit;
    m_addressBar->setFrame(false);
    m_addressBar->setClearButtonEnabled(false);
    m_addressBar->setPlaceholderText("Search the web or type a URL");
    connect(m_addressBar, &QLineEdit::returnPressed, this, [this]() { openAddress(); });

    m_starButton = new QToolButton;
    m_starButton->setObjectName("starBtn");
    m_starButton->setIcon(svgIcon("star_outline", PrimaryColor, 25));
    m_starButton->setIconSize(QSize(24, 24));
    m_starButton->setCheckable(true);
    m_starButton->setCursor(Qt::PointingHandCursor);
    connect(m_starButton, &QToolButton::toggled, this, [this](bool checked) {
        m_starButton->setIcon(svgIcon(checked ? "star_filled" : "star_outline", PrimaryColor, 25));
    });

    auto *layout = new QHBoxLayout(pill);
    layout->setContentsMargins(22, 0, 14, 0);
    layout->setSpacing(16);
    layout->addWidget(lockButton);
    layout->addWidget(m_addressBar, 1);
    layout->addWidget(m_starButton);
    return pill;
}

void BrowserWindow::buildCentral()
{
    auto *pageContainer = new QWidget;
    pageContainer->setObjectName("chromeRoot");
    auto *outerLayout = new QVBoxLayout(pageContainer);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto *shell = new QWidget;
    shell->setObjectName("appShell");
    auto *layout = new QVBoxLayout(shell);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *tabStrip = new TabStrip(this);
    tabStrip->setObjectName("tabStrip");
    tabStrip->setFixedHeight(58);
    auto *tabLayout = new QHBoxLayout(tabStrip);
    tabLayout->setContentsMargins(16, 9, 18, 0);
    tabLayout->setSpacing(10);

    m_tabButtonsContainer = new QWidget;
    m_tabButtonsContainer->setObjectName("tabButtonsContainer");
    m_tabButtonsContainer->setMaximumWidth(540);
    m_tabButtonsLayout = new QHBoxLayout(m_tabButtonsContainer);
    m_tabButtonsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabButtonsLayout->setSpacing(8);
    tabLayout->addWidget(m_tabButtonsContainer);

    auto *newTabButton = new QToolButton;
    newTabButton->setObjectName("newTabBtn");
    newTabButton->setIcon(svgIcon("add", PrimaryColor, 20));
    newTabButton->setIconSize(QSize(20, 20));
    newTabButton->setCursor(Qt::PointingHandCursor);
    newTabButton->setAutoRaise(true);
    connect(newTabButton, &QToolButton::clicked, this, [this]() { addTab(true); });
    tabLayout->addWidget(newTabButton);
    tabLayout->addStretch();

    auto *windowControls = new QWidget;
    windowControls->setObjectName("windowControls");
    auto *controlsLayout = new QHBoxLayout(windowControls);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->setSpacing(18);
    const QList<QPair<QString, std::function<void()>>> controls = {
        {"–", [this]() { minimizeWindow(); }},
        {"▢", [this]() { toggleWindowMaximized(); }},
        {"×", [this]() { close(); }},
    };
    for (const auto &control : controls) {
        auto *button = new QToolButton;
        button->setText(control.first);
        button->setProperty("windowControl", true);
        if (control.first == "×") {
            button->setObjectName("windowClose");
        }
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QToolButton::clicked, this, control.second);
        controlsLayout->addWidget(button);
    }
    tabLayout->addWidget(windowControls);

    m_pages = new QStackedWidget;
    m_pages->setObjectName("pages");

    layout->addWidget(tabStrip);
    layout->addWidget(m_chromeBar);
    layout->addWidget(m_pages);
    outerLayout->addWidget(shell);
    setCentralWidget(pageContainer);
}

BrowserTab *BrowserWindow::addTab(bool switchTo)
{
    auto *view = new BrowserTab(this);
    connect(view, &QWebEngineView::urlChanged, this, [this, view](const QUrl &url) { updateAddressBar(view, url); });
    connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) { updateTabTitle(view, title); });
    connect(view, &QWebEngineView::iconChanged, this, [this, view](const QIcon &icon) { updateTabIcon(view, icon); });
    const int index = m_pages->addWidget(view);
    auto *button = new TabButton(index, AppTitle, this);
    m_tabButtonsLayout->addWidget(button);
    loadHome(view);
    syncTabButtons();
    if (switchTo) {
        selectTab(index);
    }
    return view;
}

void BrowserWindow::closeTab(int index)
{
    if (m_pages->count() == 1) {
        loadHome(activeWebView());
        return;
    }

    QWidget *view = m_pages->widget(index);
    QWidget *button = m_tabButtonsLayout->itemAt(index)->widget();
    rememberClosedTab(view);
    if (index == m_currentTabIndex && m_pages->count() > 1) {
        const int nextIndex = index == m_pages->count() - 1 ? index - 1 : index + 1;
        selectTab(nextIndex);
    }
    m_pages->removeWidget(view);
    view->deleteLater();
    QLayoutItem *item = m_tabButtonsLayout->takeAt(index);
    if (item != nullptr) {
        if (button != nullptr) {
            button->deleteLater();
        }
        delete item;
    }
    if (m_currentTabIndex >= m_pages->count()) {
        m_currentTabIndex = m_pages->count() - 1;
    }
    if (m_pages->count() > 0) {
        selectTab(std::max(0, m_currentTabIndex));
    }
    syncTabButtons();
}

void BrowserWindow::selectTab(int index)
{
    if (index < 0 || index >= m_pages->count()) {
        return;
    }
    m_currentTabIndex = index;
    m_pages->setCurrentIndex(index);
    BrowserTab *view = activeWebView();
    const QString url = view->url().toString();
    m_addressBar->setText(url == HomeUrl ? QString() : url);
    m_starButton->setChecked(false);
    setWindowTitle(view->title().isEmpty() ? AppTitle : view->title());
    syncTabButtons();
}

void BrowserWindow::toggleWindowMaximized()
{
    isMaximized() ? showNormal() : showMaximized();
}

void BrowserWindow::syncTabButtons()
{
    for (int index = 0; index < m_tabButtonsLayout->count(); ++index) {
        auto *button = dynamic_cast<TabButton *>(m_tabButtonsLayout->itemAt(index)->widget());
        if (button != nullptr) {
            button->setIndex(index);
            button->setActive(index == m_currentTabIndex);
        }
    }
}

BrowserTab *BrowserWindow::activeWebView()
{
    auto *tab = dynamic_cast<BrowserTab *>(m_pages->currentWidget());
    if (tab == nullptr) {
        return addTab(true);
    }
    return tab;
}

void BrowserWindow::loadHome(BrowserTab *view)
{
    BrowserTab *target = view == nullptr ? activeWebView() : view;
    m_recordingHistory = false;
    target->setHtml(homeHtml(), QUrl(HomeUrl));
    m_recordingHistory = true;
    if (target == activeWebView()) {
        m_addressBar->clear();
    }
}

void BrowserWindow::showHistory()
{
    m_recordingHistory = false;
    activeWebView()->setHtml(historyHtml(), QUrl(HistoryUrl));
    m_recordingHistory = true;
    m_addressBar->setText(HistoryUrl);
}

QString BrowserWindow::historyHtml() const
{
    QString items;
    if (m_history.isEmpty()) {
        items = QStringLiteral("<p>No browsing history yet.</p>");
    } else {
        QStringList rows;
        const int first = std::max(0, m_history.size() - 50);
        for (int i = m_history.size() - 1; i >= first; --i) {
            const HistoryEntry &entry = m_history.at(i);
            const QString title = htmlEscaped(entry.title.isEmpty() ? entry.url : entry.title);
            const QString url = htmlEscaped(entry.url);
            rows << QStringLiteral(R"(<li><a href="%1"><strong>%2</strong><span>%1</span></a></li>)").arg(url, title);
        }
        items = QStringLiteral("<ol>%1</ol>").arg(rows.join(QString()));
    }
    return QStringLiteral(R"(<!doctype html>
<html><head><meta charset="utf-8"><title>History</title><style>
body { margin:0; min-height:100vh; padding:48px; font-family:Inter,Roboto,"Segoe UI",sans-serif; background:#f7fbff; color:#1e3558; }
main { max-width:820px; margin:0 auto; } h1 { font-size:44px; margin:0 0 28px; }
ol { list-style:none; margin:0; padding:0; display:grid; gap:10px; }
a { display:block; padding:16px 18px; border-radius:18px; background:#fff; color:#1e3558; text-decoration:none; box-shadow:inset 0 0 0 1px rgba(151,185,238,.28); }
a:hover { box-shadow:inset 0 0 0 1px rgba(47,126,234,.45); }
strong, span { display:block; overflow:hidden; text-overflow:ellipsis; white-space:nowrap; }
span { color:#637996; font-size:13px; margin-top:4px; }
</style></head><body><main><h1>History</h1>%1</main></body></html>)")
        .arg(items);
}

void BrowserWindow::openAddress()
{
    loadUrl(m_addressBar->text());
}

void BrowserWindow::loadUrl(const QString &rawUrl)
{
    QString url = rawUrl.trimmed();
    if (url.isEmpty()) {
        loadHome();
        return;
    }
    if (isSearchQuery(url)) {
        url = QString(GoogleSearchUrl) + QUrl::toPercentEncoding(url);
    }
    if (QUrl(url).scheme().isEmpty()) {
        url = "https://" + url;
    }
    activeWebView()->setUrl(QUrl(url));
}

void BrowserWindow::updateAddressBar(BrowserTab *view, const QUrl &url)
{
    if (view != activeWebView()) {
        return;
    }
    m_addressBar->setText(url.toString() == HomeUrl ? QString() : url.toString());
    m_starButton->setChecked(false);
}

void BrowserWindow::updateTabTitle(BrowserTab *view, const QString &title)
{
    const int index = m_pages->indexOf(view);
    if (index >= 0) {
        auto *button = dynamic_cast<TabButton *>(m_tabButtonsLayout->itemAt(index)->widget());
        if (button != nullptr) {
            button->setTitleText(title.isEmpty() ? AppTitle : title);
        }
    }
    if (view == activeWebView()) {
        setWindowTitle(title.isEmpty() ? AppTitle : title);
    }
    recordHistory(view, title);
}

void BrowserWindow::updateTabIcon(BrowserTab *view, const QIcon &icon)
{
    const int index = m_pages->indexOf(view);
    if (index < 0) {
        return;
    }
    auto *button = dynamic_cast<TabButton *>(m_tabButtonsLayout->itemAt(index)->widget());
    if (button != nullptr) {
        button->setIconPixmap(icon);
    }
}

void BrowserWindow::recordHistory(BrowserTab *view, const QString &title)
{
    if (!m_recordingHistory || view == nullptr) {
        return;
    }
    const QString url = view->url().toString();
    if (url.isEmpty() || url == HomeUrl || url == HistoryUrl) {
        return;
    }
    if (!m_history.isEmpty() && m_history.last().url == url) {
        m_history.last().title = title.isEmpty() ? url : title;
        return;
    }
    m_history.append({title.isEmpty() ? url : title, url});
}

void BrowserWindow::rememberClosedTab(QWidget *view)
{
    auto *webView = qobject_cast<QWebEngineView *>(view);
    if (webView == nullptr) {
        return;
    }
    const QString url = webView->url().toString();
    if (url.isEmpty() || url == HomeUrl) {
        return;
    }
    m_closedTabs.append(url);
    while (m_closedTabs.size() > 20) {
        m_closedTabs.removeFirst();
    }
}

void BrowserWindow::installShortcuts()
{
    auto bind = [this](const QKeySequence &sequence, const std::function<void()> &slot) {
        auto *shortcut = new QShortcut(sequence, this);
        shortcut->setContext(Qt::WindowShortcut);
        connect(shortcut, &QShortcut::activated, this, slot);
    };
    bind(QKeySequence("Ctrl+T"), [this]() { addTab(true); });
    bind(QKeySequence("Ctrl+W"), [this]() { closeCurrentTab(); });
    bind(QKeySequence("Ctrl+Shift+T"), [this]() { reopenLastClosedTab(); });
    bind(QKeySequence("Ctrl+Tab"), [this]() { cycleTab(1); });
    bind(QKeySequence("Ctrl+Shift+Tab"), [this]() { cycleTab(-1); });
    bind(QKeySequence("Ctrl+L"), [this]() { focusAddressBar(); });
    bind(QKeySequence("Ctrl+R"), [this]() { activeWebView()->reload(); });
    bind(QKeySequence("F5"), [this]() { activeWebView()->reload(); });
    bind(QKeySequence("Ctrl+Shift+R"), [this]() { hardReload(); });
    bind(QKeySequence("Ctrl++"), [this]() { zoomIn(); });
    bind(QKeySequence("Ctrl+="), [this]() { zoomIn(); });
    bind(QKeySequence("Ctrl+-"), [this]() { zoomOut(); });
    bind(QKeySequence("Ctrl+0"), [this]() { zoomReset(); });
    bind(QKeySequence("Ctrl+Q"), [this]() { close(); });
    bind(QKeySequence("F11"), [this]() { toggleFullscreen(); });
    for (int i = 1; i <= 8; ++i) {
        bind(QKeySequence(QString("Ctrl+%1").arg(i)), [this, i]() { selectTab(i - 1); });
    }
    bind(QKeySequence("Ctrl+9"), [this]() { selectTab(m_pages->count() - 1); });
}

void BrowserWindow::closeCurrentTab()
{
    if (m_currentTabIndex >= 0) {
        closeTab(m_currentTabIndex);
    }
}

void BrowserWindow::reopenLastClosedTab()
{
    if (m_closedTabs.isEmpty()) {
        return;
    }
    const QString url = m_closedTabs.takeLast();
    addTab(true)->setUrl(QUrl(url));
}

void BrowserWindow::cycleTab(int delta)
{
    const int count = m_pages->count();
    if (count == 0) {
        return;
    }
    selectTab((m_currentTabIndex + delta + count) % count);
}

void BrowserWindow::focusAddressBar()
{
    m_addressBar->setFocus(Qt::ShortcutFocusReason);
    m_addressBar->selectAll();
}

void BrowserWindow::hardReload()
{
    activeWebView()->page()->triggerAction(QWebEnginePage::ReloadAndBypassCache);
}

void BrowserWindow::zoomIn()
{
    BrowserTab *view = activeWebView();
    view->setZoomFactor(std::min(5.0, view->zoomFactor() + 0.1));
}

void BrowserWindow::zoomOut()
{
    BrowserTab *view = activeWebView();
    view->setZoomFactor(std::max(0.25, view->zoomFactor() - 0.1));
}

void BrowserWindow::zoomReset()
{
    activeWebView()->setZoomFactor(1.0);
}

void BrowserWindow::toggleFullscreen()
{
    isFullScreen() ? showNormal() : showFullScreen();
}

void BrowserWindow::minimizeWindow()
{
    showMinimized();
}

int BrowserWindow::hitTestEdge(const QPoint &globalPos) const
{
    if (isMaximized() || isFullScreen() || isMinimized()) {
        return 0;
    }
    const QRect rect = frameGeometry();
    const int x = globalPos.x() - rect.x();
    const int y = globalPos.y() - rect.y();
    int mask = 0;
    if (x < 0 || y < 0 || x >= rect.width() || y >= rect.height()) {
        return 0;
    }
    if (y < ResizeMargin) {
        mask |= Qt::TopEdge;
    }
    if (y >= rect.height() - ResizeMargin) {
        mask |= Qt::BottomEdge;
    }
    if (x < ResizeMargin) {
        mask |= Qt::LeftEdge;
    }
    if (x >= rect.width() - ResizeMargin) {
        mask |= Qt::RightEdge;
    }
    return mask;
}

Qt::CursorShape BrowserWindow::cursorForEdge(int mask) const
{
    const bool top = mask & Qt::TopEdge;
    const bool bottom = mask & Qt::BottomEdge;
    const bool left = mask & Qt::LeftEdge;
    const bool right = mask & Qt::RightEdge;
    if ((top && left) || (bottom && right)) {
        return Qt::SizeFDiagCursor;
    }
    if ((top && right) || (bottom && left)) {
        return Qt::SizeBDiagCursor;
    }
    if (left || right) {
        return Qt::SizeHorCursor;
    }
    if (top || bottom) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

void BrowserWindow::updateResizeCursor(int mask)
{
    if (mask == m_currentResizeEdge) {
        return;
    }
    m_currentResizeEdge = mask;
    if (m_hasResizeCursor) {
        QApplication::restoreOverrideCursor();
        m_hasResizeCursor = false;
    }
    if (mask != 0) {
        QApplication::setOverrideCursor(QCursor(cursorForEdge(mask)));
        m_hasResizeCursor = true;
    }
}

bool BrowserWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (m_isResizing) {
            QRect geometry = m_resizeStartGeometry;
            const QPoint delta = mouseEvent->globalPos() - m_resizeStartGlobal;
            if (m_currentResizeEdge & Qt::LeftEdge) {
                geometry.setLeft(geometry.left() + delta.x());
            }
            if (m_currentResizeEdge & Qt::RightEdge) {
                geometry.setRight(geometry.right() + delta.x());
            }
            if (m_currentResizeEdge & Qt::TopEdge) {
                geometry.setTop(geometry.top() + delta.y());
            }
            if (m_currentResizeEdge & Qt::BottomEdge) {
                geometry.setBottom(geometry.bottom() + delta.y());
            }
            if (geometry.width() >= minimumWidth() && geometry.height() >= minimumHeight()) {
                setGeometry(geometry);
            }
            return true;
        }
        updateResizeCursor(hitTestEdge(mouseEvent->globalPos()));
    } else if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton && m_currentResizeEdge != 0) {
            m_isResizing = true;
            m_resizeStartGlobal = mouseEvent->globalPos();
            m_resizeStartGeometry = geometry();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (m_isResizing) {
            m_isResizing = false;
            return true;
        }
    }
    return false;
}

void BrowserWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

}  // namespace morphine
