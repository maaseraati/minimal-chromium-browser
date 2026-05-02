#include "browserwindow.h"

#include <QApplication>
#include <QCoreApplication>

namespace {
constexpr auto kAppStyleSheet = R"QSS(
/* Material 3 dark chrome for Morphine */

QMainWindow,
QWidget#centralWidget {
    background: #10131a;
    color: #e3e2e7;
}

QStatusBar {
    background: #10131a;
    color: #c4c6cf;
    border-top: 1px solid #1b1e25;
    font-size: 12px;
}

QStatusBar::item {
    border: none;
}

QSplitter::handle {
    background: #10131a;
    width: 1px;
}

/* Tabs */
QTabWidget::pane {
    border: 0;
    background: #10131a;
}

QTabWidget#sidePanel::pane {
    border-top: 1px solid #1b1e25;
}

QTabBar {
    background: #10131a;
    qproperty-drawBase: 0;
}

QTabBar::tab {
    background: transparent;
    color: #c4c6cf;
    padding: 7px 8px 7px 14px;
    margin: 6px 2px 0;
    min-width: 140px;
    max-width: 220px;
    border: 0;
    border-radius: 12px;
    font-size: 12px;
    font-weight: 500;
}

QTabBar::tab:hover {
    background: #1b1e25;
    color: #e3e2e7;
}

QTabBar::tab:selected {
    background: #262932;
    color: #e3e2e7;
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

QTabWidget::corner {
    background: #10131a;
}

/* Tool buttons (back / forward / reload / home / +, menu) */
QToolButton {
    background: transparent;
    color: #c4c6cf;
    border: 0;
    border-radius: 14px;
    padding: 4px 8px;
    min-width: 28px;
    min-height: 28px;
    font-size: 16px;
}

QToolButton:hover {
    background: #1b1e25;
    color: #e3e2e7;
}

QToolButton:pressed {
    background: #262932;
}

QToolButton:disabled {
    color: #43474e;
}

QToolButton::menu-indicator {
    image: none;
    width: 0;
}

/* Address bar */
QLineEdit {
    background: #1b1e25;
    color: #e3e2e7;
    border: 1px solid #43474e;
    border-radius: 18px;
    padding: 6px 14px;
    selection-background-color: #284777;
    selection-color: #d8e2ff;
    font-size: 13px;
}

QLineEdit:focus {
    border-color: #aac5ff;
}

/* "Go" button (filled M3 button) */
QPushButton {
    background: #aac5ff;
    color: #00306e;
    border: 0;
    border-radius: 18px;
    padding: 7px 20px;
    font-weight: 600;
    font-size: 13px;
}

QPushButton:hover {
    background: #c2d3ff;
}

QPushButton:pressed {
    background: #92b4f5;
}

QPushButton:disabled {
    background: #262932;
    color: #43474e;
}

/* Progress bar */
QProgressBar {
    background: #1b1e25;
    border: 0;
    max-height: 2px;
}

QProgressBar::chunk {
    background: #aac5ff;
}

/* Side panel lists (bookmarks, history) */
QListWidget {
    background: #10131a;
    color: #e3e2e7;
    border: 0;
    padding: 4px;
    outline: none;
}

QListWidget::item {
    padding: 8px 10px;
    border-radius: 8px;
}

QListWidget::item:hover {
    background: #1b1e25;
}

QListWidget::item:selected {
    background: #262932;
    color: #e3e2e7;
}

/* Menus */
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

QMenu::item:selected {
    background: #262932;
}

QMenu::separator {
    height: 1px;
    background: #43474e;
    margin: 4px 8px;
}

/* Scrollbars */
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}

QScrollBar::handle:vertical {
    background: #43474e;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: #5a5e66;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    background: transparent;
}
)QSS";
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("Morphine"));
    QCoreApplication::setOrganizationName(QStringLiteral("Morphine"));

    QApplication app(argc, argv);
    app.setStyleSheet(QString::fromUtf8(kAppStyleSheet));

    BrowserWindow window;
    window.show();

    return QApplication::exec();
}
