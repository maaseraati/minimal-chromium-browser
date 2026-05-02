#include "browserwindow.h"
#include "thememanager.h"

#include <QApplication>
#include <QCoreApplication>

namespace {

// HTML prototype palette (--page is the desktop fall-through, but the chrome
// itself is rendered with the cream/green palette below):
//   --chrome-top:    #fbfcf4
//   --chrome-bottom: #f5f7ee
//   --tab-active:    #d9e5c9
//   --accent / menu: #71814f
//   --new-tab fill:  #bed09a
//   --omnibox bg:    #fbfcf7 (focus #ffffff)
//   --text:          #1e211b
//   --muted:         #555b4f
constexpr auto kStyleSheet = R"QSS(
QMainWindow,
QWidget#centralWidget {
    background: #f5f7ee;
    color: #1e211b;
}

QStatusBar {
    background: #f5f7ee;
    color: #555b4f;
    border-top: 1px solid #dfe4d6;
    font-size: 12px;
}

QStatusBar::item { border: none; }

QSplitter::handle {
    background: #dfe4d6;
    width: 1px;
}

/* Tab strip + corner widgets share the cream gradient with the tab bar. */
QTabWidget::pane {
    border: 0;
    background: #f5f7ee;
}

QTabWidget::tab-bar {
    left: 0;
}

QTabWidget::corner {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #fbfcf4, stop:1 #f6f8ef);
    border-bottom: 1px solid #dfe4d6;
}

QWidget#chromeCorner {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #fbfcf4, stop:1 #f6f8ef);
    border-bottom: 1px solid #dfe4d6;
}

QTabBar { background: transparent; qproperty-drawBase: 0; }

/* Menu chip — dark green block on the very left of the tab strip. */
QToolButton#chromeMenu {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #71814f, stop:1 #586a3e);
    color: #eef5e5;
    border: 0;
    border-bottom-right-radius: 16px;
    padding: 0;
}

QToolButton#chromeMenu:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #7c8d56, stop:1 #607143);
}

QToolButton#chromeMenu::menu-indicator { image: none; width: 0; }

/* New-tab pill on the right — light-green chip. */
QToolButton#chromeNewTab {
    background: #bed09a;
    color: #141a10;
    border: 0;
    border-radius: 14px;
    padding: 0;
}

QToolButton#chromeNewTab:hover {
    background: #c7d8a5;
}

QToolButton#chromeNewTab:pressed {
    background: #b1c587;
}

QToolButton#chromeNewTab::menu-indicator { image: none; width: 0; }

/* Window controls (—, □, ×). */
QToolButton#windowControl {
    background: transparent;
    border: 0;
    border-radius: 8px;
    color: #32372f;
}

QToolButton#windowControl:hover {
    background: rgba(76, 91, 53, 0.10);
}

QToolButton#windowControl:pressed {
    background: rgba(76, 91, 53, 0.16);
}

QToolButton#windowClose {
    background: transparent;
    border: 0;
    border-radius: 8px;
    color: #32372f;
}

QToolButton#windowClose:hover {
    background: #d4423a;
    color: #ffffff;
}

QToolButton#windowClose:pressed {
    background: #b3261e;
    color: #ffffff;
}

QToolButton#windowControl::menu-indicator,
QToolButton#windowClose::menu-indicator { image: none; width: 0; }

/* Toolbar (back/forward/reload/profile + omnibox). */
QWidget#chromeToolbar {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f7f9f1, stop:1 #f3f5ec);
    border-bottom: 1px solid #e6ead9;
}

QToolButton#navButton {
    background: transparent;
    color: #59604f;
    border: 0;
    border-radius: 24px;
    padding: 0;
}

QToolButton#navButton:hover {
    background: #eef2e7;
    color: #252a20;
}

QToolButton#navButton:pressed {
    background: #e3e9d9;
}

QToolButton#navButton:disabled {
    color: #b6bdaa;
}

QToolButton#navReload {
    background: #bed09a;
    color: #172011;
    border: 1px solid #a9bd7f;
    border-radius: 24px;
    padding: 0;
}

QToolButton#navReload:hover {
    background: #c7d8a5;
    border-color: #95ad67;
}

QToolButton#navReload:pressed {
    background: #b1c587;
}

QToolButton#profileButton {
    background: #d4e0b8;
    color: #2e3b1b;
    border: 0;
    border-radius: 24px;
    padding: 0;
}

QToolButton#profileButton:hover {
    background: #cad7ad;
}

QToolButton#profileButton:pressed {
    background: #c0cfa1;
}

QToolButton#navButton::menu-indicator,
QToolButton#navReload::menu-indicator,
QToolButton#profileButton::menu-indicator { image: none; width: 0; }

/* Omnibox — pill-shaped input matching the prototype. */
QLineEdit#omnibox {
    background: #fbfcf7;
    color: #20241d;
    border: 1px solid #dfe4d8;
    border-radius: 23px;
    padding: 6px 20px;
    selection-background-color: #d9e5c9;
    selection-color: #1e211b;
    font-size: 15px;
}

QLineEdit#omnibox:focus {
    background: #ffffff;
    border-color: #c7d8a5;
}

/* Generic line edits (find bar, settings dialog). */
QLineEdit {
    background: #fbfcf7;
    color: #20241d;
    border: 1px solid #dfe4d8;
    border-radius: 14px;
    padding: 6px 14px;
    selection-background-color: #d9e5c9;
    selection-color: #1e211b;
    font-size: 13px;
}

QLineEdit:focus {
    border-color: #c7d8a5;
}

/* Default tool buttons elsewhere (settings dialog, find bar). */
QToolButton {
    background: transparent;
    color: #59604f;
    border: 0;
    border-radius: 14px;
    padding: 4px;
    min-width: 28px;
    min-height: 28px;
}

QToolButton:hover {
    background: #eef2e7;
    color: #252a20;
}

QToolButton:pressed {
    background: #e3e9d9;
}

QToolButton#primaryAction {
    background: #71814f;
    color: #ffffff;
    border-radius: 18px;
    min-width: 36px;
    min-height: 36px;
    padding: 0;
}

QToolButton#primaryAction:hover {
    background: #7c8d56;
}

QToolButton#primaryAction:pressed {
    background: #5e6f3f;
}

QPushButton {
    background: #bed09a;
    color: #172011;
    border: 1px solid #a9bd7f;
    border-radius: 16px;
    padding: 7px 20px;
    font-weight: 600;
    font-size: 13px;
}

QPushButton:hover {
    background: #c7d8a5;
    border-color: #95ad67;
}

QPushButton:pressed {
    background: #b1c587;
}

QPushButton:disabled {
    background: #e3e9d9;
    color: #97a181;
    border-color: #d6dcc4;
}

QProgressBar {
    background: #eef2e7;
    border: 0;
    max-height: 2px;
}

QProgressBar::chunk { background: #71814f; }

/* Lists used for bookmarks / history side panel. */
QListWidget {
    background: #fbfcf4;
    color: #1e211b;
    border: 0;
    padding: 4px;
    outline: none;
}

QListWidget::item { padding: 8px 10px; border-radius: 8px; }
QListWidget::item:hover { background: #eef2e7; }
QListWidget::item:selected { background: #d9e5c9; color: #1e211b; }

QMenu {
    background: #fbfcf4;
    color: #1e211b;
    border: 1px solid #dfe4d8;
    border-radius: 12px;
    padding: 6px;
}

QMenu::item {
    background: transparent;
    padding: 7px 18px;
    border-radius: 8px;
}

QMenu::item:selected {
    background: #d9e5c9;
    color: #1e211b;
}

QMenu::separator {
    height: 1px;
    background: #dfe4d8;
    margin: 4px 8px;
}

QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}

QScrollBar::handle:vertical {
    background: #c7d0b9;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: #b3bea2;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }

QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical { background: transparent; }

/* Side panel tabs (bookmarks / history). */
QTabWidget#sidePanel::pane { background: #fbfcf4; border: 0; }
QTabWidget#sidePanel QTabBar::tab {
    background: transparent;
    color: #555b4f;
    padding: 6px 14px;
    border-radius: 12px;
    margin: 4px 2px;
    font-weight: 500;
}
QTabWidget#sidePanel QTabBar::tab:selected {
    background: #d9e5c9;
    color: #1e211b;
}
QTabWidget#sidePanel QTabBar::tab:hover:!selected {
    background: #eef2e7;
    color: #252a20;
}
)QSS";

void applyTheme(QApplication &app)
{
    app.setStyleSheet(QString::fromUtf8(kStyleSheet));
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("Morphine"));
    QCoreApplication::setOrganizationName(QStringLiteral("Morphine"));

    QApplication app(argc, argv);

    auto *theme = ThemeManager::instance();
    Q_UNUSED(theme);
    applyTheme(app);

    BrowserWindow window;
    window.show();

    return QApplication::exec();
}
