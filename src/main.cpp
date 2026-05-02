#include "browserwindow.h"
#include "thememanager.h"

#include <QApplication>
#include <QCoreApplication>

namespace {

constexpr auto kDarkStyleSheet = R"QSS(
/* Material 3 dark chrome for Morphine */

QMainWindow,
QWidget#centralWidget,
QWidget#chromeRoot {
    background: #10131a;
    color: #e3e2e7;
}

QStatusBar {
    background: #10131a;
    color: #c4c6cf;
    border-top: 1px solid #1b1e25;
    font-size: 12px;
}

QStatusBar::item { border: none; }

QSplitter::handle {
    background: #10131a;
    width: 1px;
}

QTabWidget::pane {
    border: 0;
    background: #10131a;
}

QTabBar {
    background: #10131a;
    qproperty-drawBase: 0;
    min-height: 42px;
}

QTabBar::tab {
    background: #171a22;
    color: #b9bcc7;
    padding: 9px 12px 9px 14px;
    margin: 7px 1px 0;
    min-width: 158px;
    max-width: 238px;
    min-height: 24px;
    border: 1px solid transparent;
    border-bottom: 0;
    border-top-left-radius: 15px;
    border-top-right-radius: 15px;
    font-size: 12px;
    font-weight: 500;
}

QTabBar::tab:hover {
    background: #20242e;
    color: #e3e2e7;
    border-color: #2d323d;
    margin-top: 5px;
    padding-top: 10px;
}

QTabBar::tab:selected {
    background: #262b36;
    color: #e3e2e7;
    border-color: #363c49;
    margin-top: 4px;
    padding-top: 10px;
}

QTabBar::tab:pressed {
    background: #2f3541;
}

QTabBar::close-button {
    image: url(:/assets/close.svg);
    subcontrol-position: right;
    subcontrol-origin: padding;
    width: 16px;
    height: 16px;
    margin-left: 6px;
    border-radius: 8px;
}

QTabBar::close-button:hover {
    image: url(:/assets/close-hover.svg);
    background: #43474e;
}

QTabWidget::corner { background: #10131a; }

QWidget#tabIndicator {
    background: #aac5ff;
    border-radius: 2px;
}

QToolButton {
    background: transparent;
    color: #c4c6cf;
    border: 0;
    border-radius: 16px;
    padding: 4px;
    min-width: 32px;
    min-height: 32px;
}

QTabWidget::corner QToolButton {
    border-radius: 17px;
    margin-top: 4px;
}

QToolButton:hover {
    background: #1b1e25;
    color: #e3e2e7;
}

QToolButton:pressed { background: #262932; }
QToolButton:disabled { color: #43474e; }
QToolButton::menu-indicator { image: none; width: 0; }

QToolButton#windowClose:hover {
    background: #b3261e;
    color: #ffffff;
}

QToolButton#primaryAction {
    background: #aac5ff;
    color: #00306e;
    border-radius: 18px;
    min-width: 36px;
    min-height: 36px;
    padding: 0;
}

QToolButton#primaryAction:hover { background: #c2d3ff; }
QToolButton#primaryAction:pressed { background: #92b4f5; }

QLineEdit {
    background: #1b1e25;
    color: #e3e2e7;
    border: 1px solid #1b1e25;
    border-radius: 18px;
    padding: 6px 14px 6px 10px;
    selection-background-color: #284777;
    selection-color: #d8e2ff;
    font-size: 13px;
}

QLineEdit:hover { background: #20242e; border-color: #2d323d; }
QLineEdit:focus { background: #20242e; border-color: #aac5ff; }

QPushButton {
    background: #aac5ff;
    color: #00306e;
    border: 0;
    border-radius: 18px;
    padding: 7px 20px;
    font-weight: 600;
    font-size: 13px;
}

QPushButton:hover { background: #c2d3ff; }
QPushButton:pressed { background: #92b4f5; }
QPushButton:disabled { background: #262932; color: #43474e; }

QProgressBar {
    background: #1b1e25;
    border: 0;
    max-height: 2px;
}

QProgressBar::chunk { background: #aac5ff; }

QListWidget {
    background: #10131a;
    color: #e3e2e7;
    border: 0;
    padding: 4px;
    outline: none;
}

QListWidget::item { padding: 8px 10px; border-radius: 8px; }
QListWidget::item:hover { background: #1b1e25; }
QListWidget::item:selected { background: #262932; color: #e3e2e7; }

QMenu {
    background: #1b1e25;
    color: #e3e2e7;
    border: 1px solid #43474e;
    border-radius: 10px;
    padding: 6px;
}

QMenu::item {
    background: transparent;
    padding: 7px 18px;
    border-radius: 6px;
}

QMenu::item:selected { background: #262932; }
QMenu::separator { height: 1px; background: #43474e; margin: 4px 8px; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: #43474e; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #5a5e66; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
)QSS";

constexpr auto kLightStyleSheet = R"QSS(
/* Material 3 light chrome for Morphine */

QMainWindow,
QWidget#centralWidget,
QWidget#chromeRoot {
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
    background: #fafbff;
    width: 1px;
}

QTabWidget::pane {
    border: 0;
    background: #fafbff;
}

QTabBar {
    background: #fafbff;
    qproperty-drawBase: 0;
    min-height: 42px;
}

QTabBar::tab {
    background: #eef1f7;
    color: #50545d;
    padding: 9px 12px 9px 14px;
    margin: 7px 1px 0;
    min-width: 158px;
    max-width: 238px;
    min-height: 24px;
    border: 1px solid transparent;
    border-bottom: 0;
    border-top-left-radius: 15px;
    border-top-right-radius: 15px;
    font-size: 12px;
    font-weight: 500;
}

QTabBar::tab:hover {
    background: #e4e8f0;
    color: #1a1c1f;
    border-color: #d5d9e2;
    margin-top: 5px;
    padding-top: 10px;
}

QTabBar::tab:selected {
    background: #ffffff;
    color: #1a1c1f;
    border-color: #d9dde6;
    margin-top: 4px;
    padding-top: 10px;
}

QTabBar::tab:pressed {
    background: #dce1ea;
}

QTabBar::close-button {
    image: url(:/assets/close-light.svg);
    subcontrol-position: right;
    subcontrol-origin: padding;
    width: 16px;
    height: 16px;
    margin-left: 6px;
    border-radius: 8px;
}

QTabBar::close-button:hover {
    image: url(:/assets/close-light-hover.svg);
    background: #c4c6cf;
}

QTabWidget::corner { background: #fafbff; }

QWidget#tabIndicator {
    background: #4a76b3;
    border-radius: 2px;
}

QToolButton {
    background: transparent;
    color: #44474e;
    border: 0;
    border-radius: 16px;
    padding: 4px;
    min-width: 32px;
    min-height: 32px;
}

QTabWidget::corner QToolButton {
    border-radius: 17px;
    margin-top: 4px;
}

QToolButton:hover {
    background: #eef0f7;
    color: #1a1c1f;
}

QToolButton:pressed { background: #e2e6ee; }
QToolButton:disabled { color: #c4c6cf; }
QToolButton::menu-indicator { image: none; width: 0; }

QToolButton#windowClose:hover {
    background: #b3261e;
    color: #ffffff;
}

QToolButton#primaryAction {
    background: #4a76b3;
    color: #ffffff;
    border-radius: 18px;
    min-width: 36px;
    min-height: 36px;
    padding: 0;
}

QToolButton#primaryAction:hover { background: #5e8ac9; }
QToolButton#primaryAction:pressed { background: #3a629b; }

QLineEdit {
    background: #eef0f7;
    color: #1a1c1f;
    border: 1px solid #eef0f7;
    border-radius: 18px;
    padding: 6px 14px 6px 10px;
    selection-background-color: #d8e2ff;
    selection-color: #001a41;
    font-size: 13px;
}

QLineEdit:hover { background: #e4e8f0; border-color: #d5d9e2; }
QLineEdit:focus { background: #ffffff; border-color: #4a76b3; }

QPushButton {
    background: #4a76b3;
    color: #ffffff;
    border: 0;
    border-radius: 18px;
    padding: 7px 20px;
    font-weight: 600;
    font-size: 13px;
}

QPushButton:hover { background: #5e8ac9; }
QPushButton:pressed { background: #3a629b; }
QPushButton:disabled { background: #e2e6ee; color: #c4c6cf; }

QProgressBar {
    background: #eef0f7;
    border: 0;
    max-height: 2px;
}

QProgressBar::chunk { background: #4a76b3; }

QListWidget {
    background: #fafbff;
    color: #1a1c1f;
    border: 0;
    padding: 4px;
    outline: none;
}

QListWidget::item { padding: 8px 10px; border-radius: 8px; }
QListWidget::item:hover { background: #eef0f7; }
QListWidget::item:selected { background: #e2e6ee; color: #1a1c1f; }

QMenu {
    background: #ffffff;
    color: #1a1c1f;
    border: 1px solid #c4c6cf;
    border-radius: 10px;
    padding: 6px;
}

QMenu::item {
    background: transparent;
    padding: 7px 18px;
    border-radius: 6px;
}

QMenu::item:selected { background: #eef0f7; }
QMenu::separator { height: 1px; background: #c4c6cf; margin: 4px 8px; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: #c4c6cf; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #a8aab1; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
)QSS";

void applyTheme(QApplication &app, bool light)
{
    app.setStyleSheet(QString::fromUtf8(light ? kLightStyleSheet : kDarkStyleSheet));
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("Morphine"));
    QCoreApplication::setOrganizationName(QStringLiteral("Morphine"));

    QApplication app(argc, argv);

    auto *theme = ThemeManager::instance();
    applyTheme(app, theme->isLight());
    QObject::connect(theme, &ThemeManager::lightChanged, &app,
                     [&app](bool light) { applyTheme(app, light); });

    BrowserWindow window;
    window.show();

    return QApplication::exec();
}
