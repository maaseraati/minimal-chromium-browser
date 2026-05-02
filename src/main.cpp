#include "browserwindow.h"
#include "thememanager.h"

#include <QApplication>
#include <QCoreApplication>

namespace {

// Material 3 light palette mirrored from morphine://home so chrome and the
// home page share the same accent (`#4a76b3` primary, `#d8e2ff`
// primary-container, `#001a41` on-primary-container, `#fafbff` /
// `#eef0f7` / `#e2e6ee` surface tiers, `#44474e` on-surface-variant,
// `#c4c6cf` outline-variant).
constexpr auto kStyleSheet = R"QSS(
QMainWindow,
QWidget#centralWidget {
    background: #fafbff;
    color: #1a1c1f;
}

QStatusBar {
    background: #fafbff;
    color: #44474e;
    border-top: 1px solid #e2e6ee;
    font-size: 12px;
}

QStatusBar::item { border: none; }

QSplitter::handle {
    background: #e2e6ee;
    width: 1px;
}

/* Tab strip + corner widgets share one surface so there's no banding. */
QTabWidget::pane {
    border: 0;
    background: #fafbff;
}

QTabWidget::tab-bar { left: 0; }

QTabWidget::corner {
    background: #fafbff;
}

QWidget#chromeCorner {
    background: #fafbff;
}

QTabBar { background: transparent; qproperty-drawBase: 0; }

/* Menu chip — M3 primary block at the very left of the tab strip
   (78x66 with rounded bottom-right, mirrors prototype `.menu-button`). */
QToolButton#chromeMenu {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #4a76b3, stop:1 #2e5a99);
    color: #ffffff;
    border: 0;
    border-bottom-right-radius: 16px;
    padding: 0;
    min-width: 78px;
    min-height: 66px;
    max-width: 78px;
    max-height: 66px;
}

QToolButton#chromeMenu:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #5683c0, stop:1 #3a68a8);
}

QToolButton#chromeMenu:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #3e6aa6, stop:1 #244e8c);
}

QToolButton#chromeMenu::menu-indicator { image: none; width: 0; }

/* New-tab pill on the right — light primary-container chip. 48x48 with
   14px radius, exactly like the prototype `.new-tab-button`. */
QToolButton#chromeNewTab {
    background: #d8e2ff;
    color: #001a41;
    border: 0;
    border-radius: 14px;
    padding: 0;
    min-width: 48px;
    min-height: 48px;
    max-width: 48px;
    max-height: 48px;
}

QToolButton#chromeNewTab:hover {
    background: #c5d3fa;
}

QToolButton#chromeNewTab:pressed {
    background: #b3c4f3;
}

QToolButton#chromeNewTab::menu-indicator { image: none; width: 0; }

/* Window controls (—, □, ×) — 28x28 squares with 8px radius. */
QToolButton#windowControl {
    background: transparent;
    border: 0;
    border-radius: 8px;
    color: #44474e;
    min-width: 28px;
    min-height: 28px;
    max-width: 28px;
    max-height: 28px;
}

QToolButton#windowControl:hover {
    background: rgba(74, 118, 179, 0.10);
}

QToolButton#windowControl:pressed {
    background: rgba(74, 118, 179, 0.18);
}

QToolButton#windowClose {
    background: transparent;
    border: 0;
    border-radius: 8px;
    color: #44474e;
    min-width: 28px;
    min-height: 28px;
    max-width: 28px;
    max-height: 28px;
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

/* Toolbar (back/forward/reload + omnibox + profile). Kept on the same
   surface family as the strip so the chrome reads as one block. */
QWidget#chromeToolbar {
    background: #fafbff;
    border-bottom: 1px solid #e2e6ee;
}

/* Back / forward — transparent circles, glyph only. */
QToolButton#navButton {
    background: transparent;
    color: #44474e;
    border: 0;
    border-radius: 24px;
    padding: 0;
    min-width: 48px;
    min-height: 48px;
    max-width: 48px;
    max-height: 48px;
}

QToolButton#navButton:hover {
    background: #eef0f7;
    color: #1a1c1f;
}

QToolButton#navButton:pressed {
    background: #e2e6ee;
}

QToolButton#navButton:disabled {
    color: #b6b9c1;
}

/* Reload — circular primary-container chip (M3 blue equivalent of the
   prototype's green refresh chip, same shape). */
QToolButton#navReload {
    background: #d8e2ff;
    color: #001a41;
    border: 0;
    border-radius: 24px;
    padding: 0;
    min-width: 48px;
    min-height: 48px;
    max-width: 48px;
    max-height: 48px;
}

QToolButton#navReload:hover {
    background: #c5d3fa;
}

QToolButton#navReload:pressed {
    background: #b3c4f3;
}

/* Profile — circular primary-container chip (mirrors `.profile-button`). */
QToolButton#profileButton {
    background: #d8e2ff;
    color: #001a41;
    border: 0;
    border-radius: 24px;
    padding: 0;
    min-width: 48px;
    min-height: 48px;
    max-width: 48px;
    max-height: 48px;
}

QToolButton#profileButton:hover {
    background: #c5d3fa;
}

QToolButton#profileButton:pressed {
    background: #b3c4f3;
}

QToolButton#navButton::menu-indicator,
QToolButton#navReload::menu-indicator,
QToolButton#profileButton::menu-indicator { image: none; width: 0; }

/* Omnibox — pill input mirroring the prototype `.omnibox` (height 46,
   23px radius, padding 0 20). */
QLineEdit#omnibox {
    background: #fbfcff;
    color: #1a1c1f;
    border: 1px solid #e2e6ee;
    border-radius: 23px;
    padding: 0 20px;
    selection-background-color: #d8e2ff;
    selection-color: #001a41;
    font-size: 14px;
    min-height: 46px;
    max-height: 46px;
}

QLineEdit#omnibox:focus {
    background: #ffffff;
    border-color: #4a76b3;
}

/* Generic line edits (find bar, settings dialog). */
QLineEdit {
    background: #eef0f7;
    color: #1a1c1f;
    border: 1px solid #e2e6ee;
    border-radius: 14px;
    padding: 6px 14px;
    selection-background-color: #d8e2ff;
    selection-color: #001a41;
    font-size: 13px;
}

QLineEdit:focus {
    border-color: #4a76b3;
}

/* Default tool buttons elsewhere (settings dialog, find bar). */
QToolButton {
    background: transparent;
    color: #44474e;
    border: 0;
    border-radius: 14px;
    padding: 4px;
    min-width: 28px;
    min-height: 28px;
}

QToolButton:hover {
    background: #eef0f7;
    color: #1a1c1f;
}

QToolButton:pressed {
    background: #e2e6ee;
}

QToolButton#primaryAction {
    background: #4a76b3;
    color: #ffffff;
    border-radius: 18px;
    min-width: 36px;
    min-height: 36px;
    padding: 0;
}

QToolButton#primaryAction:hover {
    background: #5683c0;
}

QToolButton#primaryAction:pressed {
    background: #3e6aa6;
}

QPushButton {
    background: #d8e2ff;
    color: #001a41;
    border: 1px solid #c1d0f5;
    border-radius: 16px;
    padding: 7px 20px;
    font-weight: 600;
    font-size: 13px;
}

QPushButton:hover {
    background: #c5d3fa;
    border-color: #a8baee;
}

QPushButton:pressed {
    background: #b3c4f3;
}

QPushButton:disabled {
    background: #eef0f7;
    color: #aab0bc;
    border-color: #e2e6ee;
}

QProgressBar {
    background: #eef0f7;
    border: 0;
    max-height: 2px;
}

QProgressBar::chunk { background: #4a76b3; }

/* Lists used for bookmarks / history side panel. */
QListWidget {
    background: #fafbff;
    color: #1a1c1f;
    border: 0;
    padding: 4px;
    outline: none;
}

QListWidget::item { padding: 8px 10px; border-radius: 8px; }
QListWidget::item:hover { background: #eef0f7; }
QListWidget::item:selected { background: #d8e2ff; color: #001a41; }

QMenu {
    background: #fafbff;
    color: #1a1c1f;
    border: 1px solid #e2e6ee;
    border-radius: 12px;
    padding: 6px;
}

QMenu::item {
    background: transparent;
    padding: 7px 18px;
    border-radius: 8px;
}

QMenu::item:selected {
    background: #d8e2ff;
    color: #001a41;
}

QMenu::separator {
    height: 1px;
    background: #e2e6ee;
    margin: 4px 8px;
}

QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}

QScrollBar::handle:vertical {
    background: #c4c6cf;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: #a9adb8;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }

QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical { background: transparent; }

/* Side panel tabs (bookmarks / history). */
QTabWidget#sidePanel::pane { background: #fafbff; border: 0; }
QTabWidget#sidePanel QTabBar::tab {
    background: transparent;
    color: #44474e;
    padding: 6px 14px;
    border-radius: 12px;
    margin: 4px 2px;
    font-weight: 500;
}
QTabWidget#sidePanel QTabBar::tab:selected {
    background: #d8e2ff;
    color: #001a41;
}
QTabWidget#sidePanel QTabBar::tab:hover:!selected {
    background: #eef0f7;
    color: #1a1c1f;
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
