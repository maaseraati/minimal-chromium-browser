#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QToolButton;
class QWebEngineProfile;
class QWebEngineView;

class BrowserTab final : public QWidget {
    Q_OBJECT

public:
    explicit BrowserTab(QWebEngineProfile *profile, QWidget *parent = nullptr);

    QWebEngineView *view() const;
    QString title() const;
    QUrl url() const;

public slots:
    void focusAddressBar();
    void loadHome();
    void loadInput(const QString &input);

signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void closeRequested();

private:
    QUrl inputToUrl(const QString &input) const;
    QString homeHtml() const;
    void updateActions();

    QWebEngineView *webView_;
    QLineEdit *addressBar_;
    QProgressBar *progressBar_;
    QToolButton *backButton_;
    QToolButton *forwardButton_;
    QToolButton *reloadButton_;
    QToolButton *homeButton_;
    QPushButton *goButton_;
    QString currentTitle_;
};
