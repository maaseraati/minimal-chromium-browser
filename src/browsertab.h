#pragma once

#include <QUrl>
#include <QWidget>

#include <QIcon>

class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QToolButton;
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineView;

class BrowserTab final : public QWidget {
    Q_OBJECT

public:
    explicit BrowserTab(
        QWebEngineProfile *profile, QWebEnginePage *page = nullptr, QWidget *parent = nullptr);

    QWebEngineView *view() const;
    QString title() const;
    QIcon icon() const;
    QUrl url() const;
    bool isLoading() const;
    bool isRestorableUrl() const;

public slots:
    void focusAddressBar();
    void findInPage(const QString &text, bool backwards = false);
    void loadHome();
    void loadInput(const QString &input);

signals:
    void iconChanged(const QIcon &icon);
    void loadingChanged(bool loading, int progress);
    void newWindowPageRequested(QWebEnginePage *page);
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void closeRequested();

private:
    QUrl inputToUrl(const QString &input) const;
    void installPage(QWebEnginePage *page);
    QString homeHtml() const;
    void setLoading(bool loading, int progress);
    void updateActions();
    void refreshIcons();
    void updatePrimaryActionIcon();
    void updateAddressLockVisible();

    QWebEngineView *webView_;
    QWidget *addressBox_;
    QLabel *lockIcon_;
    QLineEdit *addressBar_;
    QProgressBar *progressBar_;
    QToolButton *backButton_;
    QToolButton *forwardButton_;
    QToolButton *reloadButton_;
    QToolButton *primaryAction_;
    QIcon currentIcon_;
    QString currentTitle_;
    bool isLoading_;
};
