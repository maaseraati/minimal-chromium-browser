#include "chromeuibridge.h"

ChromeUiBridge::ChromeUiBridge(QObject *parent)
    : QObject(parent)
{
}

void ChromeUiBridge::pushInitialState(const QVariantMap &state)
{
    emit initialStateChanged(state);
}

void ChromeUiBridge::pushTabAdded(int index, const QVariantMap &props, bool animate)
{
    emit tabAdded(index, props, animate);
}

void ChromeUiBridge::pushTabRemoved(int index)
{
    emit tabRemoved(index);
}

void ChromeUiBridge::pushTabUpdated(int index, const QVariantMap &props)
{
    emit tabUpdated(index, props);
}

void ChromeUiBridge::pushActiveChanged(int index)
{
    emit activeChanged(index);
}

void ChromeUiBridge::pushUrlChanged(const QString &url)
{
    emit urlChanged(url);
}

void ChromeUiBridge::pushNavStateChanged(bool canBack, bool canForward, bool isLoading)
{
    emit navStateChanged(canBack, canForward, isLoading);
}

void ChromeUiBridge::pushFocusAddressBar()
{
    emit focusAddressBar();
}

void ChromeUiBridge::pushMaximizedChanged(bool maximized)
{
    emit maximizedChanged(maximized);
}

void ChromeUiBridge::requestNewTab()        { emit newTabRequested(); }
void ChromeUiBridge::requestCloseTab(int i) { emit closeTabRequested(i); }
void ChromeUiBridge::requestActivateTab(int i) { emit activateTabRequested(i); }
void ChromeUiBridge::requestReorderTab(int f, int t) { emit reorderTabRequested(f, t); }
void ChromeUiBridge::requestNavigate(const QString &i) { emit navigateRequested(i); }
void ChromeUiBridge::requestBack()          { emit backRequested(); }
void ChromeUiBridge::requestForward()       { emit forwardRequested(); }
void ChromeUiBridge::requestReload()        { emit reloadRequested(); }
void ChromeUiBridge::requestStop()          { emit stopRequested(); }
void ChromeUiBridge::requestMinimize()      { emit minimizeRequested(); }
void ChromeUiBridge::requestToggleMaximize(){ emit toggleMaximizeRequested(); }
void ChromeUiBridge::requestClose()         { emit closeRequested(); }
void ChromeUiBridge::requestMenu()          { emit menuRequested(); }
void ChromeUiBridge::requestProfile()       { emit profileRequested(); }
void ChromeUiBridge::ready()                { emit readyRequested(); }
void ChromeUiBridge::requestStartSystemMove() { emit startSystemMoveRequested(); }
void ChromeUiBridge::requestSystemDoubleClick() { emit systemDoubleClickRequested(); }
