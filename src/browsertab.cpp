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
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background: #0a0b10;
      color: #f6f7fb;
    }
    body {
      min-height: 100vh;
      margin: 0;
      display: grid;
      place-items: center;
      background:
        radial-gradient(circle at top left, rgba(125, 91, 255, .35), transparent 34rem),
        radial-gradient(circle at bottom right, rgba(0, 209, 255, .24), transparent 30rem),
        #0a0b10;
    }
    main {
      width: min(820px, calc(100vw - 40px));
      padding: 46px;
      border: 1px solid rgba(255, 255, 255, .12);
      border-radius: 34px;
      background: rgba(16, 18, 28, .76);
      box-shadow: 0 24px 90px rgba(0, 0, 0, .44);
      backdrop-filter: blur(24px);
      text-align: center;
    }
    h1 {
      margin: 0 0 8px;
      font-size: clamp(48px, 9vw, 92px);
      letter-spacing: -0.08em;
    }
    p {
      margin: 0 0 34px;
      color: #aeb5cc;
      font-size: 18px;
    }
    form {
      display: flex;
      gap: 12px;
    }
    input {
      flex: 1;
      border: 1px solid rgba(255,255,255,.16);
      border-radius: 18px;
      padding: 18px 20px;
      background: rgba(255,255,255,.08);
      color: #fff;
      font-size: 17px;
      outline: none;
    }
    input:focus {
      border-color: rgba(139, 118, 255, .9);
      box-shadow: 0 0 0 4px rgba(139, 118, 255, .18);
    }
    button {
      border: 0;
      border-radius: 18px;
      padding: 0 24px;
      background: linear-gradient(135deg, #8b76ff, #00d1ff);
      color: white;
      font-size: 16px;
      font-weight: 700;
      cursor: pointer;
    }
    .quick {
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
      gap: 10px;
      margin-top: 24px;
    }
    .quick a {
      color: #d8dcff;
      text-decoration: none;
      padding: 10px 14px;
      border-radius: 999px;
      background: rgba(255,255,255,.08);
    }
  </style>
</head>
<body>
  <main>
    <h1>morphine</h1>
    <p>Fast C++ shell around Chromium via Qt WebEngine.</p>
    <form action="https://www.google.com/search">
      <input name="q" autofocus autocomplete="off" placeholder="Search Google or type a URL">
      <button>Search</button>
    </form>
    <div class="quick">
      <a href="https://github.com">GitHub</a>
      <a href="https://news.ycombinator.com">Hacker News</a>
      <a href="https://www.qt.io/product/qt6">Qt 6</a>
    </div>
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
