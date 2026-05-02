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
    background: #11151d;
    qproperty-drawBase: 0;
    min-height: 40px;
}

QTabBar::tab {
    background: transparent;
    color: #d7dce5;
    padding: 8px 34px 8px 14px;
    margin: 0;
    min-width: 160px;
    max-width: 240px;
    min-height: 28px;
    border: 0;
    border-radius: 0;
    font-size: 13px;
    font-weight: 500;
}

QTabBar::close-button {
    image: url(:/assets/close.svg);
    subcontrol-position: right;
    subcontrol-origin: padding;
    width: 16px;
    height: 16px;
    margin-left: 8px;
    border-radius: 8px;
}

QTabBar::close-button:hover {
    image: url(:/assets/close-hover.svg);
    background: #43474e;
}

QTabWidget::corner { background: #11151d; }

QTabWidget > QWidget {
    background: #11151d;
}

QWidget#tabCorner {
    background: #11151d;
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
    border-radius: 16px;
    margin-top: 3px;
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

QWidget#addressBox {
    background: #1b1e25;
    border: 1px solid #1b1e25;
    border-radius: 18px;
}

QWidget#addressBox:hover { background: #20242e; border-color: #2d323d; }
QWidget#addressBox:focus-within { background: #20242e; border-color: #aac5ff; }

QLineEdit#addressEdit {
    background: transparent;
    color: #e3e2e7;
    border: 0;
    border-radius: 0;
    padding: 0;
    selection-background-color: #284777;
    selection-color: #d8e2ff;
    font-size: 13px;
}

QLineEdit {
    background: #1b1e25;
    color: #e3e2e7;
    border: 1px solid #1b1e25;
    border-radius: 18px;
    padding: 6px 14px;
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
/* Green prototype chrome for Morphine */

QMainWindow,
QWidget#centralWidget,
QWidget#chromeRoot {
    background: #fbfcf4;
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
    background: #fbfcf4;
    width: 1px;
}

QTabWidget::pane {
    border: 0;
    background: #fbfcf4;
}

QTabBar {
    background: #fbfcf4;
    qproperty-drawBase: 0;
    min-height: 58px;
}

QTabBar::tab {
    background: transparent;
    color: #555b4f;
    padding: 0 36px 0 18px;
    margin: 0;
    min-width: 310px;
    max-width: 310px;
    min-height: 44px;
    border: 0;
    border-radius: 15px;
    font-size: 16px;
    font-weight: 500;
}

QTabBar::close-button {
    image: url(:/assets/close-light.svg);
    subcontrol-position: right;
    subcontrol-origin: padding;
    width: 16px;
    height: 16px;
    margin-left: 8px;
    border-radius: 8px;
}

QTabBar::close-button:hover {
    image: url(:/assets/close-light-hover.svg);
    background: #c4c6cf;
}

QTabWidget::corner { background: #fbfcf4; }

QTabWidget > QWidget {
    background: #fbfcf4;
}

QWidget#tabCorner {
    background: #fbfcf4;
}

QToolButton {
    background: transparent;
    color: #59604f;
    border: 0;
    border-radius: 24px;
    padding: 4px;
    min-width: 48px;
    min-height: 48px;
}

QTabWidget::corner QToolButton {
    border-radius: 14px;
    margin-top: 5px;
}

QToolButton:hover {
    background: #eef2e7;
    color: #252a20;
}

QToolButton:pressed { background: #e3e9d9; }
QToolButton:disabled { color: #a8aea1; }
QToolButton::menu-indicator { image: none; width: 0; }

QToolButton#menuButton {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #71814f, stop:1 #586a3e);
    border-radius: 0 0 16px 0;
    min-width: 78px;
    min-height: 66px;
    max-width: 78px;
    max-height: 66px;
    padding: 0;
}

QToolButton#newTabButton {
    background: #bed09a;
    color: #141a10;
    border-radius: 14px;
    min-width: 48px;
    min-height: 48px;
    max-width: 48px;
    max-height: 48px;
}

QToolButton#newTabButton:hover { background: #c7d8a5; }
QToolButton#newTabButton:pressed { background: #b1c587; }

QToolButton#windowClose:hover {
    background: #b3261e;
    color: #ffffff;
}

QToolButton#primaryAction {
    background: #bed09a;
    color: #172011;
    border: 1px solid #a9bd7f;
    border-radius: 24px;
    min-width: 48px;
    min-height: 48px;
    padding: 0;
}

QToolButton#primaryAction:hover { background: #c7d8a5; border-color: #95ad67; }
QToolButton#primaryAction:pressed { background: #b1c587; }

QWidget#tabStrip {
    background: #fbfcf4;
    border-bottom: 1px solid #dfe4d6;
}

QWidget#browserToolbar {
    background: #f5f7ee;
    border-bottom: 0;
}

QWidget#addressBox {
    background: #fbfcf7;
    border: 1px solid #dfe4d8;
    border-radius: 23px;
}

QWidget#addressBox:hover { background: #ffffff; border-color: #c7d8a5; }
QWidget#addressBox:focus-within { background: #ffffff; border-color: #c7d8a5; }

QLineEdit#addressEdit {
    background: transparent;
    color: #20241d;
    border: 0;
    border-radius: 0;
    padding: 0;
    selection-background-color: #d9e5c9;
    selection-color: #1e211b;
    font-size: 15px;
}

QLineEdit {
    background: #fbfcf7;
    color: #20241d;
    border: 1px solid #dfe4d8;
    border-radius: 18px;
    padding: 6px 14px;
    selection-background-color: #d9e5c9;
    selection-color: #1e211b;
    font-size: 13px;
}

QLineEdit:hover { background: #ffffff; border-color: #c7d8a5; }
QLineEdit:focus { background: #ffffff; border-color: #c7d8a5; }

QPushButton {
    background: #bed09a;
    color: #141a10;
    border: 0;
    border-radius: 18px;
    padding: 7px 20px;
    font-weight: 600;
    font-size: 13px;
}

QPushButton:hover { background: #c7d8a5; }
QPushButton:pressed { background: #b1c587; }
QPushButton:disabled { background: #e3e9d9; color: #a8aea1; }

QProgressBar {
    background: #f5f7ee;
    border: 0;
    max-height: 2px;
}

QProgressBar::chunk { background: #71814f; }

QListWidget {
    background: #fbfcf4;
    color: #1e211b;
    border: 0;
    padding: 4px;
    outline: none;
}

QListWidget::item { padding: 8px 10px; border-radius: 8px; }
QListWidget::item:hover { background: #edf1e2; }
QListWidget::item:selected { background: #d9e5c9; color: #1e211b; }

QMenu {
    background: #ffffff;
    color: #1e211b;
    border: 1px solid #dfe4d6;
    border-radius: 10px;
    padding: 6px;
}

QMenu::item {
    background: transparent;
    padding: 7px 18px;
    border-radius: 6px;
}

QMenu::item:selected { background: #edf1e2; }
QMenu::separator { height: 1px; background: #dfe4d6; margin: 4px 8px; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: #c7d8a5; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #b9cc93; }
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
