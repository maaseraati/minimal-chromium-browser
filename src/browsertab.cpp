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
      --accent-soft: #203452;
      --bg: #07101f;
      --surface: #101b2d;
      --surface-high: #17243a;
      --outline: #263850;
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
        radial-gradient(circle at 14% 14%, #17355c 0, transparent 25rem),
        radial-gradient(circle at 86% 18%, #132848 0, transparent 29rem),
        radial-gradient(circle at 65% 96%, #12305d 0, transparent 34rem),
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
      opacity: .22;
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
      border: 1px solid #17325c;
      border-radius: 50%;
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
      padding: 32px 0 54px;
    }
    .topbar {
      display: flex;
      justify-content: flex-end;
      align-items: center;
      gap: 16px;
      margin-bottom: 52px;
    }
    .settings-chip {
      display: grid;
      place-items: center;
      width: 40px;
      height: 40px;
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
      margin-bottom: 14px;
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
      border: 1px solid var(--outline);
      border-radius: 28px;
      background: var(--surface);
      box-shadow:
        0 20px 70px rgba(0, 0, 0, .28),
        inset 0 1px 0 rgba(255, 255, 255, .08);
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
      margin: 36px auto 0;
    }
    .shortcut {
      display: grid;
      justify-content: center;
      gap: 10px;
      min-height: 82px;
      padding: 11px 10px;
      border: 1px solid var(--outline);
      border-radius: 24px;
      background: var(--surface-high);
      color: #dde9fb;
      box-shadow: 0 16px 44px rgba(0, 0, 0, .16);
    }
    .shortcut:hover {
      border-color: #3b5b84;
      background: #1c2c46;
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
    }
    @media (max-width: 520px) {
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
