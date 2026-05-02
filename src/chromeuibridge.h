#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

// Bridge object exposed to the chrome UI HTML via QWebChannel.
//
// JS calls public slots when the user clicks a button, drags a tab, edits the
// URL, etc. Each slot re-emits a request signal that BrowserWindow connects to.
//
// BrowserWindow calls pushXxx() methods to ship new state to JS — those simply
// emit the JS-facing signals declared below.
class ChromeUiBridge final : public QObject {
    Q_OBJECT

public:
    explicit ChromeUiBridge(QObject *parent = nullptr);

    void pushInitialState(const QVariantMap &state);
    void pushTabAdded(int index, const QVariantMap &props, bool animate);
    void pushTabRemoved(int index);
    void pushTabUpdated(int index, const QVariantMap &props);
    void pushActiveChanged(int index);
    void pushUrlChanged(const QString &url);
    void pushNavStateChanged(bool canBack, bool canForward, bool isLoading);
    void pushFocusAddressBar();
    void pushMaximizedChanged(bool maximized);

public slots:
    // Invoked from JavaScript.
    void requestNewTab();
    void requestCloseTab(int index);
    void requestStartSystemMove();
    void requestSystemDoubleClick();
    void requestActivateTab(int index);
    void requestReorderTab(int from, int to);
    void requestNavigate(const QString &input);
    void requestBack();
    void requestForward();
    void requestReload();
    void requestStop();
    void requestMinimize();
    void requestToggleMaximize();
    void requestClose();
    void requestMenu();
    void requestProfile();
    void ready();

signals:
    // Pushed to JavaScript (connected by chrome_ui.html).
    void initialStateChanged(QVariantMap state);
    void tabAdded(int index, QVariantMap props, bool animate);
    void tabRemoved(int index);
    void tabUpdated(int index, QVariantMap props);
    void activeChanged(int index);
    void urlChanged(QString url);
    void navStateChanged(bool canBack, bool canForward, bool isLoading);
    void focusAddressBar();
    void maximizedChanged(bool maximized);

    // Re-emitted to BrowserWindow.
    void newTabRequested();
    void closeTabRequested(int index);
    void activateTabRequested(int index);
    void reorderTabRequested(int from, int to);
    void navigateRequested(QString input);
    void backRequested();
    void forwardRequested();
    void reloadRequested();
    void stopRequested();
    void minimizeRequested();
    void toggleMaximizeRequested();
    void closeRequested();
    void menuRequested();
    void profileRequested();
    void readyRequested();
    void startSystemMoveRequested();
    void systemDoubleClickRequested();
};
