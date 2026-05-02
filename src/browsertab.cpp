#include "browsertab.h"

#include "browserpage.h"
#include "iconutils.h"
#include "settingsdialog.h"
#include "thememanager.h"

#include <QAction>
#include <QColor>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLineEdit>
#include <QProgressBar>
#include <QShortcut>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace {
constexpr auto kHomeUrl = "morphine://home";

auto makeToolButton(const QString &toolTip) -> QToolButton *
{
    auto *button = new QToolButton;
    button->setToolTip(toolTip);
    button->setAutoRaise(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setIconSize(QSize(20, 20));
    return button;
}
} // namespace

BrowserTab::BrowserTab(QWebEngineProfile *profile, QWebEnginePage *page, QWidget *parent)
    : QWidget(parent),
      webView_(new QWebEngineView(this)),
      addressBar_(new QLineEdit(this)),
      progressBar_(new QProgressBar(this)),
      backButton_(makeToolButton("Back")),
      forwardButton_(makeToolButton("Forward")),
      reloadButton_(makeToolButton("Reload")),
      primaryAction_(makeToolButton("Go")),
      lockAction_(nullptr),
      currentTitle_("Morphine"),
      isLoading_(false)
{
    installPage(page ? page : new BrowserPage(profile, webView_));

    addressBar_->setClearButtonEnabled(false);
    addressBar_->setPlaceholderText("Enter URL or search Google");
    addressBar_->setMinimumHeight(36);
    lockAction_ = addressBar_->addAction(QIcon(), QLineEdit::LeadingPosition);
    updateAddressLockVisible();

    progressBar_->setTextVisible(false);
    progressBar_->setMaximumHeight(2);
    progressBar_->hide();

    primaryAction_->setObjectName(QStringLiteral("primaryAction"));
    primaryAction_->setIconSize(QSize(20, 20));
    primaryAction_->setFixedSize(QSize(36, 36));

    refreshIcons();
    connect(ThemeManager::instance(), &ThemeManager::lightChanged, this,
            [this](bool) { refreshIcons(); });

    auto *toolbar = new QHBoxLayout;
    toolbar->setContentsMargins(12, 8, 12, 8);
    toolbar->setSpacing(4);
    toolbar->addWidget(backButton_);
    toolbar->addWidget(forwardButton_);
    toolbar->addWidget(reloadButton_);
    toolbar->addSpacing(4);
    toolbar->addWidget(addressBar_, 1);
    toolbar->addSpacing(4);
    toolbar->addWidget(primaryAction_);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(toolbar);
    layout->addWidget(progressBar_);
    layout->addWidget(webView_, 1);

    auto *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("morphineTheme"), ThemeManager::instance());
    webView_->page()->setWebChannel(channel);

    connect(backButton_, &QToolButton::clicked, webView_, &QWebEngineView::back);
    connect(forwardButton_, &QToolButton::clicked, webView_, &QWebEngineView::forward);
    connect(reloadButton_, &QToolButton::clicked, this, [this] {
        if (isLoading_) {
            webView_->stop();
        } else {
            webView_->reload();
        }
    });
    connect(primaryAction_, &QToolButton::clicked, this, [this] {
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
        updateAddressLockVisible();
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
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Home), this, SLOT(loadHome()));

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
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }
    * {
      box-sizing: border-box;
    }
    html,
    body {
      margin: 0;
      background: #10131a;
      color: #e3e2e7;
    }
    a {
      color: inherit;
      text-decoration: none;
    }
    .theme-toggle {
      position: absolute;
      opacity: 0;
      pointer-events: none;
    }
    .stage {
      --primary: #aac5ff;
      --on-primary: #00306e;
      --primary-container: #284777;
      --on-primary-container: #d8e2ff;
      --surface: #10131a;
      --surface-container: #1b1e25;
      --surface-container-high: #262932;
      --on-surface: #e3e2e7;
      --on-surface-variant: #c4c6cf;
      --outline-variant: #43474e;
      --wave-from: rgba(170, 197, 255, .22);
      --wave-mid: rgba(170, 197, 255, .08);
      --wave-to: rgba(170, 197, 255, 0);
      --star-color: rgba(170, 197, 255, .55);
      --shadow: 0 24px 60px rgba(0, 0, 0, .35);

      position: relative;
      min-height: 100vh;
      overflow: hidden;
      background: var(--surface);
      color: var(--on-surface);
      transition: background .4s ease, color .4s ease;
    }
    .stage.light {
      --primary: #4a76b3;
      --on-primary: #ffffff;
      --primary-container: #d8e2ff;
      --on-primary-container: #001a41;
      --surface: #fafbff;
      --surface-container: #eef0f7;
      --surface-container-high: #e2e6ee;
      --on-surface: #1a1c1f;
      --on-surface-variant: #44474e;
      --outline-variant: #c4c6cf;
      --wave-from: rgba(74, 118, 179, .18);
      --wave-mid: rgba(74, 118, 179, .06);
      --wave-to: rgba(74, 118, 179, 0);
      --star-color: rgba(74, 118, 179, .42);
      --shadow: 0 18px 48px rgba(74, 118, 179, .14);
    }
    .wave {
      position: fixed;
      left: 0;
      right: 0;
      bottom: 0;
      height: 58vh;
      pointer-events: none;
      z-index: 0;
      background: linear-gradient(to top,
        var(--wave-from) 0%,
        var(--wave-mid) 42%,
        var(--wave-to) 100%);
      -webkit-mask-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1440 600" preserveAspectRatio="none"><path d="M0,470 C220,360 460,540 720,440 C980,340 1200,520 1440,420 L1440,600 L0,600 Z" fill="black"/></svg>');
      -webkit-mask-size: 100% 100%;
      -webkit-mask-repeat: no-repeat;
      mask-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1440 600" preserveAspectRatio="none"><path d="M0,470 C220,360 460,540 720,440 C980,340 1200,520 1440,420 L1440,600 L0,600 Z" fill="black"/></svg>');
      mask-size: 100% 100%;
      mask-repeat: no-repeat;
      transition: background .4s ease;
    }
    .star {
      position: fixed;
      pointer-events: none;
      z-index: 0;
      color: var(--star-color);
      transition: color .4s ease;
      animation: twinkle 7s ease-in-out infinite;
    }
    .star svg {
      width: 100%;
      height: 100%;
      display: block;
      fill: currentColor;
    }
    .star.s1 {
      width: 96px;
      height: 96px;
      right: 8vw;
      top: 13vh;
    }
    .star.s2 {
      width: 56px;
      height: 56px;
      right: 22vw;
      top: 30vh;
      animation-duration: 8.5s;
      animation-delay: -1.6s;
    }
    .star.s3 {
      width: 38px;
      height: 38px;
      left: 11vw;
      top: 26vh;
      animation-duration: 6.4s;
      animation-delay: -3s;
    }
    .star.s4 {
      width: 28px;
      height: 28px;
      left: 28vw;
      top: 12vh;
      animation-duration: 5.4s;
      animation-delay: -2.2s;
    }
    @keyframes twinkle {
      0%, 100% { transform: rotate(0) scale(1); opacity: .85; }
      50%      { transform: rotate(35deg) scale(1.06); opacity: 1; }
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
      gap: 12px;
      margin-bottom: 52px;
    }
    .settings-chip {
      display: grid;
      place-items: center;
      width: 40px;
      height: 40px;
      color: var(--primary);
      font-size: 19px;
      transition: color .4s ease;
    }
    .theme-label {
      position: relative;
      display: inline-grid;
      grid-template-columns: 1fr 1fr;
      align-items: center;
      width: 72px;
      height: 38px;
      border: 1px solid var(--outline-variant);
      border-radius: 999px;
      background: var(--surface-container);
      color: var(--on-surface-variant);
      cursor: pointer;
      transition: background .4s ease, border-color .4s ease, color .4s ease;
    }
    .theme-label::before {
      position: absolute;
      content: "";
      width: 30px;
      height: 30px;
      left: 4px;
      top: 3px;
      border-radius: 50%;
      background: var(--primary);
      transition: transform .32s ease, background .4s ease;
    }
    .theme-label span {
      position: relative;
      z-index: 1;
      display: grid;
      place-items: center;
      font-size: 14px;
    }
    .stage.light .theme-label::before {
      transform: translateX(32px);
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
      filter: drop-shadow(0 18px 36px rgba(80, 129, 196, .2));
      transition: filter .4s ease;
    }
    .headline {
      max-width: 630px;
      margin: 0 0 22px;
      color: var(--on-surface-variant);
      font-size: clamp(15px, 2vw, 18px);
      line-height: 1.6;
      letter-spacing: .01em;
      transition: color .4s ease;
    }
    form {
      display: flex;
      align-items: center;
      gap: 12px;
      width: min(700px, 100%);
      min-height: 60px;
      padding: 6px 6px 6px 18px;
      border: 1px solid var(--outline-variant);
      border-radius: 32px;
      background: var(--surface-container);
      color: var(--on-surface);
      box-shadow: var(--shadow);
      transition: background .4s ease, border-color .4s ease, color .4s ease, box-shadow .4s ease;
    }
    .search-icon,
    .voice {
      display: grid;
      place-items: center;
      flex: 0 0 40px;
      height: 40px;
      color: var(--on-surface-variant);
      transition: color .4s ease;
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
      color: var(--on-surface);
      font-size: 16px;
      outline: none;
      transition: color .4s ease;
    }
    input::placeholder {
      color: var(--on-surface-variant);
      transition: color .4s ease;
    }
    button {
      border: 0;
      border-radius: 24px;
      min-height: 48px;
      padding: 0 22px;
      background: var(--primary);
      color: var(--on-primary);
      font-size: 15px;
      font-weight: 600;
      letter-spacing: .01em;
      cursor: pointer;
      transition: background .4s ease, color .4s ease;
    }
    .shortcuts {
      display: grid;
      grid-template-columns: repeat(6, minmax(62px, 1fr));
      gap: 10px;
      width: min(550px, 100%);
      margin: 28px auto 0;
    }
    .shortcut {
      display: grid;
      justify-items: center;
      gap: 6px;
      min-height: 70px;
      padding: 12px 8px;
      border: 1px solid var(--outline-variant);
      border-radius: 22px;
      background: var(--surface-container-high);
      color: var(--on-surface);
      transition: background .4s ease, border-color .4s ease, color .4s ease;
    }
    .shortcut:hover {
      border-color: var(--primary-container);
      background: var(--primary-container);
      color: var(--on-primary-container);
    }
    .shortcut-icon {
      display: grid;
      place-items: center;
      width: 32px;
      height: 32px;
      border-radius: 14px;
      background: var(--primary-container);
      color: var(--on-primary-container);
      font-size: 16px;
      font-weight: 700;
      transition: background .4s ease, color .4s ease;
    }
    .shortcut span:last-child {
      font-size: 12px;
      font-weight: 600;
      color: var(--on-surface-variant);
      transition: color .4s ease;
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
        border-radius: 26px;
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
  <input class="theme-toggle" id="theme-toggle" type="checkbox" aria-label="Light theme">
  <div class="stage" id="stage">
    <div class="wave" aria-hidden="true"></div>
    <span class="star s1" aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M12 1 C13 8 16 11 23 12 C16 13 13 16 12 23 C11 16 8 13 1 12 C8 11 11 8 12 1 Z"/></svg></span>
    <span class="star s2" aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M12 1 C13 8 16 11 23 12 C16 13 13 16 12 23 C11 16 8 13 1 12 C8 11 11 8 12 1 Z"/></svg></span>
    <span class="star s3" aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M12 1 C13 8 16 11 23 12 C16 13 13 16 12 23 C11 16 8 13 1 12 C8 11 11 8 12 1 Z"/></svg></span>
    <span class="star s4" aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M12 1 C13 8 16 11 23 12 C16 13 13 16 12 23 C11 16 8 13 1 12 C8 11 11 8 12 1 Z"/></svg></span>
    <main class="page">
      <nav class="topbar" aria-label="Morphine shortcuts">
        <label class="theme-label" for="theme-toggle" title="Toggle light theme">
          <span>☾</span>
          <span>☼</span>
        </label>
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
  </div>
  <script src="qrc:/qtwebchannel/qwebchannel.js"></script>
  <script>
    (function() {
      var stage = document.getElementById('stage');
      var toggle = document.getElementById('theme-toggle');
      function applyTheme(light) {
        stage.classList.toggle('light', !!light);
        toggle.checked = !!light;
      }
      new QWebChannel(qt.webChannelTransport, function(channel) {
        var bridge = channel.objects.morphineTheme;
        applyTheme(bridge.light);
        bridge.lightChanged.connect(function(light) { applyTheme(light); });
        toggle.addEventListener('change', function(e) {
          bridge.setLight(e.target.checked);
        });
      });
    })();
  </script>
</body>
</html>
)HTML");
}

void BrowserTab::updateActions()
{
    backButton_->setEnabled(webView_->history()->canGoBack());
    forwardButton_->setEnabled(webView_->history()->canGoForward());
    reloadButton_->setToolTip(isLoading_ ? QStringLiteral("Stop") : QStringLiteral("Reload"));
    refreshIcons();
}

void BrowserTab::refreshIcons()
{
    const bool light = ThemeManager::instance()->isLight();
    const QColor iconColor(light ? QStringLiteral("#44474e") : QStringLiteral("#c4c6cf"));
    backButton_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/arrow-back.svg"), iconColor));
    forwardButton_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/arrow-forward.svg"), iconColor));
    reloadButton_->setIcon(IconUtils::coloredSvg(
        isLoading_ ? QStringLiteral(":/assets/stop.svg") : QStringLiteral(":/assets/refresh.svg"),
        iconColor));
    if (lockAction_) {
        lockAction_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/lock.svg"), iconColor, 18));
    }
    updatePrimaryActionIcon();
}

void BrowserTab::updatePrimaryActionIcon()
{
    const bool light = ThemeManager::instance()->isLight();
    const QColor onPrimary(light ? QStringLiteral("#ffffff") : QStringLiteral("#00306e"));
    primaryAction_->setIcon(IconUtils::coloredSvg(QStringLiteral(":/assets/arrow-go.svg"), onPrimary, 22));
}

void BrowserTab::updateAddressLockVisible()
{
    if (!lockAction_) {
        return;
    }
    const QString scheme = webView_->url().scheme();
    lockAction_->setVisible(scheme == QStringLiteral("https") || scheme == QStringLiteral("morphine"));
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
