from __future__ import annotations

import sys

from PyQt6.QtCore import QUrl
from PyQt6.QtWidgets import (
    QApplication,
    QHBoxLayout,
    QLineEdit,
    QMainWindow,
    QPushButton,
    QToolBar,
    QVBoxLayout,
    QWidget,
)
from PyQt6.QtWebEngineWidgets import QWebEngineView


HOME_URL = "https://example.com"


class BrowserWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()

        self.setWindowTitle("Minimal Chromium Browser")
        self.resize(1200, 800)

        self.web_view = QWebEngineView()
        self.address_bar = QLineEdit()
        self.address_bar.setClearButtonEnabled(True)
        self.address_bar.setPlaceholderText("Enter URL")
        self.address_bar.returnPressed.connect(self.open_address)

        back_button = QPushButton("Back")
        back_button.clicked.connect(self.web_view.back)

        forward_button = QPushButton("Forward")
        forward_button.clicked.connect(self.web_view.forward)

        reload_button = QPushButton("Reload")
        reload_button.clicked.connect(self.web_view.reload)

        go_button = QPushButton("Go")
        go_button.clicked.connect(self.open_address)

        toolbar = QToolBar("Navigation")
        toolbar.setMovable(False)

        toolbar_container = QWidget()
        toolbar_layout = QHBoxLayout(toolbar_container)
        toolbar_layout.setContentsMargins(4, 4, 4, 4)
        toolbar_layout.addWidget(back_button)
        toolbar_layout.addWidget(forward_button)
        toolbar_layout.addWidget(reload_button)
        toolbar_layout.addWidget(self.address_bar, stretch=1)
        toolbar_layout.addWidget(go_button)
        toolbar.addWidget(toolbar_container)
        self.addToolBar(toolbar)

        page_container = QWidget()
        page_layout = QVBoxLayout(page_container)
        page_layout.setContentsMargins(0, 0, 0, 0)
        page_layout.addWidget(self.web_view)
        self.setCentralWidget(page_container)

        self.web_view.urlChanged.connect(self.update_address_bar)
        self.web_view.titleChanged.connect(self.update_window_title)

        self.load_url(HOME_URL)

    def open_address(self) -> None:
        self.load_url(self.address_bar.text())

    def load_url(self, raw_url: str) -> None:
        url = raw_url.strip()
        if not url:
            return

        if not QUrl(url).scheme():
            url = f"https://{url}"

        self.web_view.setUrl(QUrl(url))

    def update_address_bar(self, url: QUrl) -> None:
        self.address_bar.setText(url.toString())

    def update_window_title(self, title: str) -> None:
        self.setWindowTitle(title or "Minimal Chromium Browser")


def main() -> int:
    app = QApplication(sys.argv)
    window = BrowserWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
