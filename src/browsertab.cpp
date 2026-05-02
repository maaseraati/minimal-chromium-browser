#include "browsertab.h"

#include "browserpage.h"
#include "settingsdialog.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QShortcut>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace {
constexpr auto kHomeUrl = "morphine://home";

auto makeToolButton(const QString &text, const QString &toolTip) -> QToolButton *
{
    auto *button = new QToolButton;
    button->setText(text);
    button->setToolTip(toolTip);
    button->setAutoRaise(true);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}
} // namespace

BrowserTab::BrowserTab(QWebEngineProfile *profile, QWebEnginePage *page, QWidget *parent)
    : QWidget(parent),
      webView_(new QWebEngineView(this)),
      addressBar_(new QLineEdit(this)),
      progressBar_(new QProgressBar(this)),
      backButton_(makeToolButton("‹", "Back")),
      forwardButton_(makeToolButton("›", "Forward")),
      reloadButton_(makeToolButton("⟳", "Reload")),
      homeButton_(makeToolButton("⌂", "Home")),
      goButton_(new QPushButton("Go", this)),
      currentTitle_("Morphine"),
      isLoading_(false)
{
    installPage(page ? page : new BrowserPage(profile, webView_));

    addressBar_->setClearButtonEnabled(true);
    addressBar_->setPlaceholderText("Enter URL or search Google");

    progressBar_->setTextVisible(false);
    progressBar_->setMaximumHeight(2);
    progressBar_->hide();

    goButton_->setCursor(Qt::PointingHandCursor);

    auto *toolbar = new QHBoxLayout;
    toolbar->setContentsMargins(10, 8, 10, 6);
    toolbar->setSpacing(8);
    toolbar->addWidget(backButton_);
    toolbar->addWidget(forwardButton_);
    toolbar->addWidget(reloadButton_);
    toolbar->addWidget(homeButton_);
    toolbar->addWidget(addressBar_, 1);
    toolbar->addWidget(goButton_);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(toolbar);
    layout->addWidget(progressBar_);
    layout->addWidget(webView_, 1);

    connect(backButton_, &QToolButton::clicked, webView_, &QWebEngineView::back);
    connect(forwardButton_, &QToolButton::clicked, webView_, &QWebEngineView::forward);
    connect(reloadButton_, &QToolButton::clicked, this, [this] {
        if (isLoading_) {
            webView_->stop();
        } else {
            webView_->reload();
        }
    });
    connect(homeButton_, &QToolButton::clicked, this, &BrowserTab::loadHome);
    connect(goButton_, &QPushButton::clicked, this, [this] {
        loadInput(addressBar_->text());
    });
    connect(addressBar_, &QLineEdit::returnPressed, this, [this] {
        loadInput(addressBar_->text());
    });

    connect(webView_, &QWebEngineView::titleChanged, this, [this](const QString &title) {
        currentTitle_ = title.isEmpty() ? QStringLiteral("New tab") : title;
        emit titleChanged(currentTitle_);
    });
    connect(webView_, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
        if (url.toString() != kHomeUrl) {
            addressBar_->setText(url.toString());
        }
        updateActions();
        emit urlChanged(url);
    });
    connect(webView_, &QWebEngineView::loadProgress, this, [this](int progress) {
        progressBar_->setValue(progress);
        setLoading(progress > 0 && progress < 100, progress);
    });
    connect(webView_, &QWebEngineView::loadStarted, this, [this] {
        setLoading(true, 0);
    });
    connect(webView_, &QWebEngineView::loadFinished, this, [this] {
        setLoading(false, 100);
        updateActions();
    });

    new QShortcut(QKeySequence::Refresh, webView_, SLOT(reload()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this, SLOT(focusAddressBar()));

    loadHome();
}

QWebEngineView *BrowserTab::view() const
{
    return webView_;
}

QString BrowserTab::title() const
{
    return currentTitle_;
}

QIcon BrowserTab::icon() const
{
    return currentIcon_;
}

QUrl BrowserTab::url() const
{
    return webView_->url();
}

bool BrowserTab::isLoading() const
{
    return isLoading_;
}

bool BrowserTab::isRestorableUrl() const
{
    const QUrl currentUrl = url();
    return currentUrl.isValid() && currentUrl.toString() != kHomeUrl;
}

void BrowserTab::focusAddressBar()
{
    addressBar_->setFocus();
    addressBar_->selectAll();
}

void BrowserTab::findInPage(const QString &text, bool backwards)
{
    if (text.isEmpty()) {
        webView_->page()->findText(QString());
        return;
    }

    QWebEnginePage::FindFlags flags;
    if (backwards) {
        flags |= QWebEnginePage::FindBackward;
    }

    webView_->page()->findText(text, flags);
}

void BrowserTab::loadHome()
{
    const QString configured = SettingsDialog::homeUrl();
    if (configured == QString::fromLatin1(kHomeUrl)) {
        currentTitle_ = QStringLiteral("Morphine");
        addressBar_->clear();
        webView_->setHtml(homeHtml(), QUrl(kHomeUrl));
        emit titleChanged(currentTitle_);
        return;
    }

    webView_->load(QUrl::fromUserInput(configured));
}

void BrowserTab::loadInput(const QString &input)
{
    const QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        loadHome();
        return;
    }

    webView_->load(inputToUrl(trimmed));
}

QUrl BrowserTab::inputToUrl(const QString &input) const
{
    const bool looksLikeUrl = input.contains('.') && !input.contains(' ');
    QUrl url(input);

    if (url.isValid() && !url.scheme().isEmpty()) {
        return url;
    }

    if (looksLikeUrl) {
        return QUrl(QStringLiteral("https://") + input);
    }

    QString templateUrl = SettingsDialog::searchUrl();
    if (!templateUrl.contains(QStringLiteral("%1"))) {
        templateUrl = SettingsDialog::defaultSearchUrl();
    }
    const QString encoded = QString::fromLatin1(QUrl::toPercentEncoding(input));
    return QUrl(templateUrl.arg(encoded));
}

void BrowserTab::installPage(QWebEnginePage *page)
{
    page->setParent(webView_);
    webView_->setPage(page);

    if (auto *browserPage = qobject_cast<BrowserPage *>(page)) {
        connect(browserPage, &BrowserPage::newWindowPageCreated, this, [this](QWebEnginePage *newPage) {
            emit newWindowPageRequested(newPage);
        });
    }

    connect(page, &QWebEnginePage::iconChanged, this, [this](const QIcon &icon) {
        currentIcon_ = icon;
        emit iconChanged(icon);
    });
}

QString BrowserTab::homeHtml() const
{
    return QStringLiteral(R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Morphine</title>
  <style>
    :root {
      color-scheme: dark;
      --accent: #8fb9ef;
      --accent-strong: #6f9fdf;
      --accent-soft: rgba(143, 185, 239, .18);
      --bg: #07101f;
      --surface: rgba(25, 34, 52, .72);
      --surface-high: rgba(37, 48, 70, .78);
      --outline: rgba(196, 214, 244, .16);
      --text: #eef5ff;
      --muted: #a8b7ce;
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background: var(--bg);
      color: var(--text);
    }
    * {
      box-sizing: border-box;
    }
    body {
      min-height: 100vh;
      margin: 0;
      overflow-x: hidden;
      background:
        radial-gradient(circle at 13% 12%, rgba(143, 185, 239, .36), transparent 24rem),
        radial-gradient(circle at 88% 18%, rgba(74, 108, 171, .28), transparent 28rem),
        radial-gradient(circle at 65% 98%, rgba(32, 66, 125, .42), transparent 34rem),
        linear-gradient(135deg, #07101f 0%, #0b1728 48%, #101e32 100%);
    }
    body::before,
    body::after {
      position: fixed;
      content: "";
      pointer-events: none;
    }
    body::before {
      inset: 0;
      opacity: .4;
      background:
        linear-gradient(90deg, rgba(255, 255, 255, .035) 1px, transparent 1px),
        linear-gradient(0deg, rgba(255, 255, 255, .025) 1px, transparent 1px);
      background-size: 72px 72px;
      mask-image: linear-gradient(to bottom, black, transparent 78%);
    }
    body::after {
      width: 720px;
      height: 720px;
      right: -210px;
      top: -210px;
      border: 1px solid rgba(143, 185, 239, .18);
      border-radius: 50%;
      box-shadow: inset 0 0 80px rgba(143, 185, 239, .08);
    }
    a {
      color: inherit;
      text-decoration: none;
    }
    .page {
      position: relative;
      z-index: 1;
      width: min(1120px, calc(100vw - 48px));
      min-height: 100vh;
      margin: 0 auto;
      padding: 30px 0;
    }
    .topbar {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      margin-bottom: 24px;
    }
    .brand-chip,
    .settings-chip {
      display: inline-flex;
      align-items: center;
      gap: 10px;
      min-height: 42px;
      padding: 0 16px;
      border: 1px solid var(--outline);
      border-radius: 999px;
      background: rgba(16, 25, 42, .62);
      color: #d8e7ff;
      box-shadow: 0 14px 44px rgba(0, 0, 0, .16);
      backdrop-filter: blur(20px);
    }
    .brand-mark {
      width: 28px;
      height: 20px;
      border-radius: 10px;
      background:
        radial-gradient(circle at 72% 28%, #bcd8ff 0 28%, transparent 29%),
        linear-gradient(135deg, #cfe2ff, var(--accent-strong));
      box-shadow: inset 0 -8px 16px rgba(32, 82, 155, .3);
    }
    .settings-chip {
      width: 42px;
      justify-content: center;
      padding: 0;
      color: var(--accent);
      font-size: 19px;
    }
    .hero {
      display: grid;
      justify-items: center;
      text-align: center;
    }
    .logo {
      width: min(390px, 68vw);
      height: auto;
      margin-bottom: 12px;
      filter: drop-shadow(0 20px 42px rgba(80, 129, 196, .22));
    }
    .headline {
      max-width: 630px;
      margin: 0 0 20px;
      color: var(--muted);
      font-size: clamp(15px, 2vw, 18px);
      line-height: 1.6;
      letter-spacing: .01em;
    }
    form {
      display: flex;
      align-items: center;
      gap: 12px;
      width: min(700px, 100%);
      min-height: 58px;
      padding: 8px;
      border: 1px solid rgba(210, 226, 255, .18);
      border-radius: 28px;
      background: linear-gradient(180deg, rgba(38, 49, 71, .82), rgba(24, 33, 51, .74));
      box-shadow:
        0 20px 70px rgba(0, 0, 0, .28),
        inset 0 1px 0 rgba(255, 255, 255, .08);
      backdrop-filter: blur(26px);
    }
    .search-icon,
    .voice {
      display: grid;
      place-items: center;
      flex: 0 0 44px;
      height: 44px;
      color: #c8d7ec;
    }
    .search-icon svg,
    .voice svg {
      width: 22px;
      height: 22px;
      stroke: currentColor;
      stroke-width: 2;
      fill: none;
      stroke-linecap: round;
      stroke-linejoin: round;
    }
    input {
      flex: 1;
      min-width: 0;
      border: 0;
      padding: 0;
      background: transparent;
      color: var(--text);
      font-size: 16px;
      outline: none;
    }
    input::placeholder {
      color: #aebbd0;
    }
    button {
      border: 0;
      border-radius: 22px;
      min-height: 48px;
      padding: 0 22px;
      background: linear-gradient(135deg, #d5e6ff, var(--accent) 52%, #6f9fdf);
      color: #071427;
      font-size: 15px;
      font-weight: 700;
      cursor: pointer;
      box-shadow: 0 12px 30px rgba(111, 159, 223, .24);
    }
    .shortcuts {
      display: grid;
      grid-template-columns: repeat(6, minmax(86px, 1fr));
      gap: 14px;
      width: min(760px, 100%);
      margin: 34px auto 0;
    }
    .shortcut {
      display: grid;
      justify-content: center;
      gap: 10px;
      min-height: 82px;
      padding: 11px 10px;
      border: 1px solid rgba(209, 225, 255, .12);
      border-radius: 24px;
      background: linear-gradient(180deg, rgba(41, 54, 77, .72), rgba(25, 34, 52, .7));
      color: #dde9fb;
      box-shadow: 0 16px 44px rgba(0, 0, 0, .16);
      backdrop-filter: blur(18px);
      transition: transform .18s ease, border-color .18s ease, background .18s ease;
    }
    .shortcut:hover {
      transform: translateY(-3px);
      border-color: rgba(143, 185, 239, .34);
      background: linear-gradient(180deg, rgba(61, 78, 108, .78), rgba(29, 41, 63, .76));
    }
    .shortcut-icon {
      display: grid;
      place-items: center;
      width: 42px;
      height: 42px;
      margin: 0 auto;
      border-radius: 16px;
      background: var(--accent-soft);
      color: #d8e8ff;
      font-size: 21px;
      font-weight: 800;
    }
    .shortcut span:last-child {
      font-size: 13px;
      font-weight: 650;
    }
    .content-grid {
      display: grid;
      grid-template-columns: .95fr 1.35fr;
      gap: 16px;
      margin-top: 30px;
    }
    .panel {
      border: 1px solid rgba(209, 225, 255, .14);
      border-radius: 30px;
      background: linear-gradient(180deg, rgba(25, 35, 55, .74), rgba(14, 23, 38, .74));
      box-shadow:
        0 22px 80px rgba(0, 0, 0, .22),
        inset 0 1px 0 rgba(255, 255, 255, .06);
      backdrop-filter: blur(22px);
      overflow: hidden;
    }
    .panel-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 20px 22px 10px;
    }
    .panel-title {
      margin: 0;
      font-size: 15px;
      letter-spacing: .01em;
    }
    .panel-action {
      color: var(--accent);
      font-size: 13px;
      font-weight: 700;
    }
    .recent-list,
    .discover-list {
      display: grid;
      gap: 8px;
      padding: 10px 16px 16px;
    }
    .recent-item,
    .discover-row {
      display: grid;
      align-items: center;
      gap: 12px;
      border-radius: 20px;
      background: rgba(255, 255, 255, .035);
    }
    .recent-item {
      grid-template-columns: 44px 1fr auto;
      padding: 10px;
    }
    .favicon {
      display: grid;
      place-items: center;
      width: 44px;
      height: 44px;
      border-radius: 15px;
      background: rgba(143, 185, 239, .16);
      color: #d7e8ff;
      font-weight: 800;
    }
    .item-title,
    .discover-title {
      color: #eef5ff;
      font-size: 14px;
      font-weight: 680;
    }
    .item-meta,
    .discover-meta {
      margin-top: 3px;
      color: #8fa1bb;
      font-size: 12px;
    }
    .item-time {
      color: #8294ae;
      font-size: 12px;
    }
    .feature-card {
      display: grid;
      gap: 16px;
      padding: 0 16px 16px;
    }
    .feature-art {
      min-height: 136px;
      border-radius: 24px;
      background:
        radial-gradient(circle at 52% 35%, rgba(225, 238, 255, .9), transparent 0 46px),
        radial-gradient(circle at 39% 43%, rgba(143, 185, 239, .9), transparent 0 76px),
        radial-gradient(circle at 64% 64%, rgba(51, 101, 183, .96), transparent 0 94px),
        linear-gradient(135deg, #0d3f99, #8ebcff);
      box-shadow: inset 0 1px 0 rgba(255, 255, 255, .14);
    }
    .discover-copy {
      display: grid;
      grid-template-columns: 1fr auto;
      gap: 14px;
      align-items: end;
    }
    .discover-copy h3 {
      margin: 0;
      font-size: clamp(19px, 3vw, 25px);
      letter-spacing: -.03em;
    }
    .discover-copy p {
      max-width: 520px;
      margin: 7px 0 0;
      color: var(--muted);
      font-size: 14px;
      line-height: 1.55;
    }
    .discover-list {
      grid-template-columns: repeat(3, 1fr);
      padding-top: 0;
    }
    .discover-row {
      padding: 13px;
    }
    @media (max-width: 860px) {
      .page {
        width: min(680px, calc(100vw - 32px));
        padding: 28px 0;
      }
      .topbar {
        margin-bottom: 34px;
      }
      form {
        border-radius: 24px;
      }
      button {
        display: none;
      }
      .shortcuts {
        grid-template-columns: repeat(3, 1fr);
      }
      .content-grid,
      .discover-copy,
      .discover-list {
        grid-template-columns: 1fr;
      }
    }
    @media (max-width: 520px) {
      .brand-chip {
        font-size: 13px;
      }
      .search-icon,
      .voice {
        flex-basis: 36px;
      }
      .shortcuts {
        grid-template-columns: repeat(2, 1fr);
      }
    }
  </style>
</head>
<body>
  <main class="page">
    <nav class="topbar" aria-label="Morphine shortcuts">
      <a class="brand-chip" href="morphine://home" aria-label="Morphine home">
        <span class="brand-mark" aria-hidden="true"></span>
        <span>Morphine</span>
      </a>
      <a class="settings-chip" href="morphine://settings" aria-label="Settings">⚙</a>
    </nav>

    <section class="hero" aria-label="Search">
      <img class="logo" src="qrc:/assets/morphine-logo.png" alt="Morphine">
      <p class="headline">Fast Chromium shell with a calm Material You home surface.</p>
    <form action="https://www.google.com/search">
        <span class="search-icon" aria-hidden="true">
          <svg viewBox="0 0 24 24"><circle cx="11" cy="11" r="7"></circle><path d="m16.5 16.5 4 4"></path></svg>
        </span>
        <input name="q" autocomplete="off" placeholder="Search the web or type a URL">
        <span class="voice" aria-hidden="true">
          <svg viewBox="0 0 24 24"><path d="M12 4a3 3 0 0 0-3 3v5a3 3 0 0 0 6 0V7a3 3 0 0 0-3-3Z"></path><path d="M5 11a7 7 0 0 0 14 0"></path><path d="M12 18v3"></path></svg>
        </span>
        <button>Search</button>
    </form>

      <div class="shortcuts">
        <a class="shortcut" href="https://www.google.com"><span class="shortcut-icon">G</span><span>Google</span></a>
        <a class="shortcut" href="https://www.youtube.com"><span class="shortcut-icon">▶</span><span>YouTube</span></a>
        <a class="shortcut" href="https://x.com"><span class="shortcut-icon">𝕏</span><span>X</span></a>
        <a class="shortcut" href="https://github.com"><span class="shortcut-icon">⌘</span><span>GitHub</span></a>
        <a class="shortcut" href="https://www.reddit.com"><span class="shortcut-icon">r</span><span>Reddit</span></a>
        <a class="shortcut" href="https://www.wikipedia.org"><span class="shortcut-icon">W</span><span>Wikipedia</span></a>
      </div>
    </section>

    <section class="content-grid" aria-label="Home content">
      <article class="panel">
        <div class="panel-header">
          <h2 class="panel-title">Continue where you left off</h2>
          <a class="panel-action" href="https://www.google.com/search?q=material+design+3">Explore</a>
        </div>
        <div class="recent-list">
          <a class="recent-item" href="https://m3.material.io">
            <span class="favicon">M3</span>
            <span><span class="item-title">Material Design 3</span><span class="item-meta">m3.material.io</span></span>
            <span class="item-time">2h</span>
          </a>
          <a class="recent-item" href="https://developer.chrome.com">
            <span class="favicon">C</span>
            <span><span class="item-title">Chrome for Developers</span><span class="item-meta">developer.chrome.com</span></span>
            <span class="item-time">1d</span>
          </a>
          <a class="recent-item" href="https://doc.qt.io/qt-6/qtwebengine-index.html">
            <span class="favicon">Qt</span>
            <span><span class="item-title">Qt WebEngine</span><span class="item-meta">doc.qt.io</span></span>
            <span class="item-time">3d</span>
          </a>
        </div>
      </article>

      <article class="panel">
        <div class="panel-header">
          <h2 class="panel-title">Discover</h2>
          <a class="panel-action" href="https://m3.material.io">Show more</a>
        </div>
        <div class="feature-card">
          <div class="feature-art" aria-hidden="true"></div>
          <div class="discover-copy">
            <div>
              <h3>Material You for Morphine</h3>
              <p>Soft surfaces, deep blue glass layers, and rounded controls tuned for a compact browser start page.</p>
            </div>
            <a class="panel-action" href="https://m3.material.io">Read</a>
          </div>
        </div>
        <div class="discover-list">
          <a class="discover-row" href="https://m3.material.io/styles/color/overview">
            <span class="discover-title">Designing with color</span>
            <span class="discover-meta">m3.material.io</span>
          </a>
          <a class="discover-row" href="https://developer.android.com/design/ui/mobile/guides/styles/color">
            <span class="discover-title">Build better with Material</span>
            <span class="discover-meta">developer.android.com</span>
          </a>
          <a class="discover-row" href="https://web.dev/learn/design">
            <span class="discover-title">Responsive UI basics</span>
            <span class="discover-meta">web.dev</span>
          </a>
        </div>
      </article>
    </section>
  </main>
</body>
</html>
)HTML");
}

void BrowserTab::updateActions()
{
    backButton_->setEnabled(webView_->history()->canGoBack());
    forwardButton_->setEnabled(webView_->history()->canGoForward());
    reloadButton_->setText(isLoading_ ? QStringLiteral("×") : QStringLiteral("⟳"));
    reloadButton_->setToolTip(isLoading_ ? QStringLiteral("Stop") : QStringLiteral("Reload"));
}

void BrowserTab::setLoading(bool loading, int progress)
{
    if (isLoading_ == loading && progressBar_->value() == progress) {
        return;
    }

    isLoading_ = loading;
    progressBar_->setValue(progress);
    progressBar_->setVisible(loading);
    updateActions();
    emit loadingChanged(loading, progress);
}
