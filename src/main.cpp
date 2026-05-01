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
#include <QPainter>
#include <QPixmap>
#include <QShortcut>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QStyle>
#include <QSvgRenderer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWindow>
#include <QWidget>

#include <algorithm>
#include <functional>

namespace {

constexpr auto AppTitle = "morphine";
constexpr auto HomeUrl = "morphine://home";
constexpr auto HistoryUrl = "morphine://history";
constexpr auto GoogleSearchUrl = "https://www.google.com/search?q=";

constexpr auto PrimaryColor = "#2f7eea";
constexpr auto OnSurface = "#1e3558";
constexpr auto OnSurfaceVariant = "#476285";

struct HistoryEntry {
    QString title;
    QString url;
};

QString iconSvg(const QString &name, const QString &color)
{
    if (name == "arrow_back") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "arrow_forward") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="m12 4-1.41 1.41L16.17 11H4v2h12.17l-5.58 5.59L12 20l8-8z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "refresh") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M17.65 6.35A7.95 7.95 0 0 0 12 4a8 8 0 1 0 7.45 5h-2.1A6 6 0 1 1 12 6c1.66 0 3.14.69 4.22 1.78L13 11h8V3z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "history") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M13 3a9 9 0 1 1-8.95 8H2l3.1-3.1L8.2 11H6.07A7 7 0 1 0 13 5a6.96 6.96 0 0 0-4.95 2.05L6.64 5.64A8.96 8.96 0 0 1 13 3zm-1 4h1.5v5l4 2.4-.75 1.23L12 12.8z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "lock") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M17 9h-1V7a4 4 0 0 0-8 0v2H7a2 2 0 0 0-2 2v8a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-8a2 2 0 0 0-2-2zm-7-2a2 2 0 1 1 4 0v2h-4zm7 12H7v-8h10z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "star_outline") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="m22 9.24-7.19-.62L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21 12 17.27 18.18 21l-1.63-7.03zM12 15.4l-3.76 2.27 1-4.28-3.32-2.88 4.38-.38L12 6.1l1.71 4.04 4.38.38-3.32 2.88 1 4.28z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "star_filled") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 17.27 18.18 21l-1.64-7.03L22 9.24l-7.19-.61L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "more_vert") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 8a2 2 0 1 0 0-4 2 2 0 0 0 0 4zm0 2a2 2 0 1 0 0 4 2 2 0 0 0 0-4zm0 6a2 2 0 1 0 0 4 2 2 0 0 0 0-4z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "search") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M9.5 3a6.5 6.5 0 0 1 5.18 10.43l.27.27h.8l5 5-1.5 1.5-5-5v-.8l-.27-.27A6.5 6.5 0 1 1 9.5 3zm0 2a4.5 4.5 0 1 0 0 9 4.5 4.5 0 0 0 0-9z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "github") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 2a10 10 0 0 0-3.16 19.49c.5.09.68-.22.68-.48v-1.7c-2.78.6-3.37-1.18-3.37-1.18-.45-1.15-1.11-1.46-1.11-1.46-.91-.62.07-.61.07-.61 1 .07 1.53 1.03 1.53 1.03.9 1.52 2.34 1.08 2.91.82.09-.65.35-1.08.63-1.33-2.22-.25-4.56-1.11-4.56-4.95 0-1.1.39-1.99 1.03-2.69-.1-.25-.45-1.27.1-2.65 0 0 .84-.27 2.75 1.02A9.5 9.5 0 0 1 12 6.98c.85 0 1.7.11 2.5.33 1.9-1.29 2.74-1.02 2.74-1.02.55 1.38.2 2.4.1 2.65.64.7 1.03 1.6 1.03 2.69 0 3.85-2.34 4.69-4.57 4.94.36.31.68.92.68 1.86V21c0 .27.18.58.69.48A10 10 0 0 0 12 2z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "close") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M18.3 5.71 12 12l6.3 6.29-1.41 1.41-6.3-6.29-6.3 6.29-1.41-1.41L9.17 12 2.88 5.71 4.29 4.3l6.3 6.29 6.29-6.29z"/></svg>)SVG")
            .arg(color);
    }
    return QString::fromUtf8(
               R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6z"/></svg>)SVG")
        .arg(color);
}

QIcon svgIcon(const QString &name, const QString &color = PrimaryColor, int size = 24)
{
    QSvgRenderer renderer(iconSvg(name, color).toUtf8());
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    return QIcon(pixmap);
}

QString htmlEscaped(const QString &value)
{
    QString result = value.toHtmlEscaped();
    result.replace('"', "&quot;");
    return result;
}

bool isSearchQuery(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.contains(' ')) {
        return true;
    }
    return !trimmed.contains('.') && !trimmed.contains(':');
}

QString homeHtml()
{
    return QStringLiteral(R"(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>morphine</title>
  <style>
    * { box-sizing: border-box; }
    :root {
      --accent: #2f7eea;
      --accent-soft: #d8e9ff;
      --accent-mid: #7fb3ff;
      --text: #1e3558;
      --muted: #7186a3;
    }
    body {
      margin: 0;
      min-height: 100vh;
      overflow: hidden;
      display: flex;
      align-items: flex-start;
      justify-content: center;
      padding: 10vh 24px 24px;
      font-family: Inter, Roboto, "Segoe UI", sans-serif;
      color: var(--text);
      background:
        radial-gradient(900px 360px at 50% 13%, rgba(47, 126, 234, .13), transparent 68%),
        linear-gradient(174deg, rgba(255,255,255,.98) 0%, rgba(248,251,255,.98) 48%, rgba(217,235,255,.72) 49%, rgba(237,247,255,.96) 100%);
    }
    body::before {
      content: "";
      position: fixed;
      inset: auto -8vw -20vh -8vw;
      height: 56vh;
      background: linear-gradient(136deg, rgba(47, 126, 234, .08), rgba(47, 126, 234, .22));
      clip-path: polygon(0 30%, 100% 0, 100% 100%, 0 100%);
      pointer-events: none;
    }
    main {
      position: relative;
      width: min(720px, 100%);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 30px;
      z-index: 1;
    }
    h1 {
      margin: 0;
      display: flex;
      align-items: center;
      gap: 4px;
      font-size: clamp(70px, 10vw, 104px);
      letter-spacing: -.055em;
      line-height: 1;
      font-weight: 850;
      color: var(--accent-mid);
      text-shadow: 0 10px 24px rgba(47, 126, 234, .18);
    }
    .brand-mark {
      width: .9em;
      height: .64em;
      display: inline-block;
      border-radius: .18em;
      background:
        radial-gradient(circle at 28% 62%, rgba(32, 96, 198, .48) 0 18%, transparent 19%),
        linear-gradient(135deg, rgba(127,179,255,.88), rgba(47,126,234,.64));
      box-shadow: 0 14px 28px rgba(47, 126, 234, .22);
      clip-path: polygon(0 8%, 16% 0, 42% 32%, 58% 32%, 84% 0, 100% 8%, 100% 92%, 84% 100%, 58% 68%, 42% 68%, 16% 100%, 0 92%);
    }
    form {
      width: min(670px, 100%);
      height: 74px;
      display: flex;
      gap: 14px;
      align-items: center;
      padding: 8px 8px 8px 28px;
      border-radius: 999px;
      background: rgba(255,255,255,.92);
      box-shadow: 0 18px 42px rgba(70, 113, 185, .14), inset 0 0 0 1px rgba(143, 178, 232, .28);
      backdrop-filter: blur(16px);
    }
    .search-icon {
      width: 24px;
      height: 24px;
      color: #7c8ba2;
      flex-shrink: 0;
    }
    input {
      flex: 1;
      min-width: 0;
      height: 56px;
      border: 0;
      outline: none;
      background: transparent;
      color: var(--text);
      font: inherit;
      font-size: 16px;
      font-weight: 520;
    }
    input::placeholder { color: #7f8ea5; }
    button {
      width: 52px;
      height: 52px;
      border: 0;
      border-radius: 999px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      background: linear-gradient(135deg, #73abff, var(--accent));
      color: white;
      font: inherit;
      font-size: 0;
      font-weight: 800;
      cursor: pointer;
      box-shadow: 0 10px 22px rgba(47, 126, 234, .32);
    }
    button::before {
      content: "➜";
      font-size: 28px;
      line-height: 1;
    }
    .chips {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 10px;
      padding: 12px;
      border-radius: 26px;
      background: rgba(255,255,255,.58);
      box-shadow: inset 0 0 0 1px rgba(143,178,232,.18);
      backdrop-filter: blur(14px);
    }
    .chip {
      height: 42px;
      display: inline-flex;
      align-items: center;
      gap: 9px;
      padding: 0 18px;
      border-radius: 999px;
      background: rgba(255,255,255,.66);
      color: #5c6d86;
      text-decoration: none;
      font-size: 15px;
      font-weight: 650;
      box-shadow: inset 0 0 0 1px rgba(143,178,232,.20), 0 8px 18px rgba(70,113,185,.08);
    }
    .chip:hover { background: #fff; color: var(--text); }
    .chip-icon {
      width: 20px;
      height: 20px;
      display: inline-grid;
      place-items: center;
      border-radius: 50%;
      background: rgba(47,126,234,.12);
      color: var(--accent);
      font-size: 13px;
      font-weight: 850;
    }
    .chip.add {
      width: 48px;
      padding: 0;
      justify-content: center;
      font-size: 25px;
      color: var(--accent);
    }
  </style>
</head>
<body>
  <main>
    <h1><span class="brand-mark"></span><span>orphine</span></h1>
    <form action="https://www.google.com/search" method="get" role="search">
      <svg class="search-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M9.5 3a6.5 6.5 0 0 1 5.18 10.43l.27.27h.8l5 5-1.5 1.5-5-5v-.8l-.27-.27A6.5 6.5 0 1 1 9.5 3zm0 2a4.5 4.5 0 1 0 0 9 4.5 4.5 0 0 0 0-9z"/></svg>
      <input name="q" type="search" placeholder="Search the web or type a URL" autofocus autocomplete="off">
      <button type="submit">Search</button>
    </form>
    <nav class="chips" aria-label="Quick links">
      <a class="chip" href="https://www.google.com"><span class="chip-icon">G</span>Google</a>
      <a class="chip" href="https://github.com"><span class="chip-icon">⌘</span>GitHub</a>
      <a class="chip" href="https://news.ycombinator.com"><span class="chip-icon">Y</span>Hacker News</a>
      <a class="chip" href="https://wikipedia.org"><span class="chip-icon">W</span>Wikipedia</a>
      <a class="chip add" href="morphine://home" aria-label="Add shortcut">+</a>
    </nav>
  </main>
</body>
</html>)");
}

}  // namespace

class BrowserWindow;

class BrowserTab : public QWebEngineView {
public:
    explicit BrowserTab(BrowserWindow *window);

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    BrowserWindow *m_window;
};

class TabButton : public QWidget {
public:
    explicit TabButton(int index, const QString &title, BrowserWindow *window);

    void setActive(bool active);
    void setTitleText(const QString &title);
    void setIconPixmap(const QIcon &icon);
    int index() const { return m_index; }
    void setIndex(int index) { m_index = index; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_index;
    BrowserWindow *m_window;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QToolButton *m_closeButton;
    bool m_active = false;
    bool m_hasSiteIcon = false;
};

class TabStrip : public QWidget {
public:
    explicit TabStrip(BrowserWindow *window);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    bool isDragTarget(const QPoint &pos) const;
    void beginWindowMove();

    BrowserWindow *m_window;
    QPoint m_pressPos;
    QPoint m_pressGlobal;
    bool m_pressed = false;
    bool m_dragging = false;
};

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

BrowserTab::BrowserTab(BrowserWindow *window) : QWebEngineView(static_cast<QWidget *>(window)), m_window(window) {}

QWebEngineView *BrowserTab::createWindow(QWebEnginePage::WebWindowType)
{
    return m_window->addTab(true);
}

TabButton::TabButton(int index, const QString &title, BrowserWindow *window)
    : m_index(index), m_window(window)
{
    setObjectName("browserTab");
    setFixedHeight(54);
    setMinimumWidth(0);
    setMaximumWidth(265);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(22, 0, 16, 0);
    layout->setSpacing(10);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("tabBadge");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(24, 24);
    m_iconLabel->hide();
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("tabTitle");
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    layout->addWidget(m_titleLabel, 1);

    m_closeButton = new QToolButton(this);
    m_closeButton->setObjectName("tabCloseBtn");
    m_closeButton->setIcon(svgIcon("close", "#ffffff", 15));
    m_closeButton->setIconSize(QSize(13, 13));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAutoRaise(true);
    m_closeButton->hide();
    connect(m_closeButton, &QToolButton::clicked, this, [this]() { m_window->closeTab(m_index); });
    layout->addWidget(m_closeButton);
}

void TabButton::setActive(bool active)
{
    m_active = active;
    setProperty("active", active);
    style()->unpolish(this);
    style()->polish(this);
    if (active) {
        m_closeButton->show();
    } else if (!underMouse()) {
        m_closeButton->hide();
    }
}

void TabButton::setTitleText(const QString &title)
{
    m_titleLabel->setText(title);
    if (title.compare("History", Qt::CaseInsensitive) == 0 && !m_hasSiteIcon) {
        m_iconLabel->setPixmap(svgIcon("history", "#ffffff", 22).pixmap(22, 22));
        m_iconLabel->show();
    }
}

void TabButton::setIconPixmap(const QIcon &icon)
{
    if (icon.isNull()) {
        m_hasSiteIcon = false;
        m_iconLabel->clear();
        m_iconLabel->hide();
        return;
    }
    const QPixmap pixmap = icon.pixmap(16, 16);
    if (pixmap.isNull()) {
        m_hasSiteIcon = false;
        m_iconLabel->clear();
        m_iconLabel->hide();
        return;
    }
    m_hasSiteIcon = true;
    m_iconLabel->setPixmap(pixmap);
    m_iconLabel->show();
}

void TabButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_window->selectTab(m_index);
    }
    QWidget::mousePressEvent(event);
}

void TabButton::enterEvent(QEvent *event)
{
    m_closeButton->show();
    QWidget::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event)
{
    if (!m_active) {
        m_closeButton->hide();
    }
    QWidget::leaveEvent(event);
}

TabStrip::TabStrip(BrowserWindow *window) : m_window(window) {}

bool TabStrip::isDragTarget(const QPoint &pos) const
{
    QWidget *widget = childAt(pos);
    while (widget != nullptr && widget != this) {
        if (dynamic_cast<TabButton *>(widget) != nullptr || qobject_cast<QToolButton *>(widget) != nullptr ||
            qobject_cast<QLineEdit *>(widget) != nullptr) {
            return false;
        }
        widget = widget->parentWidget();
    }
    return true;
}

void TabStrip::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragTarget(event->pos())) {
        m_pressed = true;
        m_pressPos = event->pos();
        m_pressGlobal = event->globalPos();
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void TabStrip::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed && (event->buttons() & Qt::LeftButton) && !m_dragging) {
        const QPoint delta = event->globalPos() - m_pressGlobal;
        if (delta.manhattanLength() >= QApplication::startDragDistance()) {
            m_dragging = true;
            beginWindowMove();
            event->accept();
            return;
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TabStrip::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void TabStrip::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragTarget(event->pos())) {
        m_window->toggleWindowMaximized();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TabStrip::beginWindowMove()
{
    if (m_window->isMaximized()) {
        m_window->showNormal();
    }
    if (m_window->windowHandle() != nullptr) {
        m_window->windowHandle()->startSystemMove();
    }
}

BrowserWindow::BrowserWindow()
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowTitle(AppTitle);
    resize(1200, 800);
    setMouseTracking(true);
    setStyleSheet(QStringLiteral(R"(
QMainWindow, QWidget#chromeRoot, QWidget#appShell, QStackedWidget#pages { background: #f7fbff; }
QWidget#tabStrip { background: #f9fcff; border-top: 1px solid #d9e8fb; border-left: 1px solid #d9e8fb; border-right: 1px solid #d9e8fb; border-top-left-radius: 18px; border-top-right-radius: 18px; }
QWidget#chromeBar { background: #edf6ff; border: 1px solid rgba(150,184,236,.36); border-top: 0; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; }
QWidget#browserTab { background: transparent; border: 0; border-radius: 0; color: #476285; }
QWidget#browserTab:hover { background: rgba(47,126,234,.06); border-top-left-radius: 15px; border-top-right-radius: 15px; }
QWidget#browserTab[active="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #73abff, stop:1 #4e8ff0); border-top-left-radius: 15px; border-top-right-radius: 15px; color: #fff; }
QLabel#tabBadge { color: #ffffff; font-size: 14px; font-weight: 700; }
QLabel#tabTitle { color: #243d60; font-size: 15px; font-weight: 600; }
QWidget#browserTab[active="true"] QLabel#tabTitle { color: #fff; }
QToolButton#tabCloseBtn { background: transparent; border: 0; border-radius: 10px; min-width: 20px; min-height: 20px; max-width: 20px; max-height: 20px; }
QToolButton#tabCloseBtn:hover { background: rgba(255,255,255,.28); }
QToolButton#newTabBtn { background: rgba(47,126,234,.08); border: 0; border-radius: 22px; min-width: 44px; min-height: 44px; max-width: 44px; max-height: 44px; }
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
    tabStrip->setFixedHeight(66);
    auto *tabLayout = new QHBoxLayout(tabStrip);
    tabLayout->setContentsMargins(0, 10, 24, 0);
    tabLayout->setSpacing(16);

    m_tabButtonsContainer = new QWidget;
    m_tabButtonsLayout = new QHBoxLayout(m_tabButtonsContainer);
    m_tabButtonsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabButtonsLayout->setSpacing(0);
    tabLayout->addWidget(m_tabButtonsContainer);

    auto *newTabButton = new QToolButton;
    newTabButton->setObjectName("newTabBtn");
    newTabButton->setIcon(svgIcon("add", PrimaryColor, 20));
    newTabButton->setIconSize(QSize(22, 22));
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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(AppTitle);
    BrowserWindow window;
    window.show();
    return app.exec();
}
