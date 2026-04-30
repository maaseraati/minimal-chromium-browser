from __future__ import annotations

import sys
from urllib.parse import quote_plus

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


APP_TITLE = "morphine"
HOME_URL = "morphine://home"
GOOGLE_SEARCH_URL = "https://www.google.com/search?q="
HOME_HTML = """
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>morphine</title>
  <style>
    :root {
      color-scheme: light;
      font-family: Arial, Helvetica, sans-serif;
      background: #f7f7f8;
      color: #111827;
    }

    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      align-items: flex-start;
      justify-content: center;
    }

    main {
      width: min(720px, calc(100% - 32px));
      margin-top: 18vh;
      text-align: center;
    }

    h1 {
      margin: 0 0 28px;
      font-size: clamp(52px, 9vw, 92px);
      line-height: 1;
      font-weight: 900;
      letter-spacing: -0.08em;
      text-transform: lowercase;
    }

    form {
      display: flex;
      gap: 10px;
      width: 100%;
    }

    input {
      flex: 1;
      height: 52px;
      border: 1px solid #d1d5db;
      border-radius: 999px;
      padding: 0 22px;
      font-size: 17px;
      outline: none;
      background: #ffffff;
      box-shadow: 0 16px 40px rgba(15, 23, 42, 0.08);
    }

    input:focus {
      border-color: #111827;
    }

    button {
      height: 52px;
      border: 0;
      border-radius: 999px;
      padding: 0 24px;
      font-size: 16px;
      font-weight: 700;
      color: #ffffff;
      background: #111827;
      cursor: pointer;
    }

    button:hover {
      background: #000000;
    }
  </style>
</head>
<body>
  <main>
    <h1>morphine</h1>
    <form action="https://www.google.com/search" method="get">
      <input name="q" type="search" placeholder="Search Google" autofocus>
      <button type="submit">Search</button>
    </form>
  </main>
</body>
</html>
"""


class BrowserWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()

        self.setWindowTitle(APP_TITLE)
        self.resize(1200, 800)

        self.web_view = QWebEngineView()
        self.address_bar = QLineEdit()
        self.address_bar.setClearButtonEnabled(True)
        self.address_bar.setPlaceholderText("Search Google or enter URL")
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

        self.load_home()

    def open_address(self) -> None:
        self.load_url(self.address_bar.text())

    def load_home(self) -> None:
        self.web_view.setHtml(HOME_HTML, QUrl(HOME_URL))
        self.address_bar.setText("")

    def load_url(self, raw_url: str) -> None:
        url = raw_url.strip()
        if not url:
            self.load_home()
            return

        if self.is_search_query(url):
            url = f"{GOOGLE_SEARCH_URL}{quote_plus(url)}"

        if not QUrl(url).scheme():
            url = f"https://{url}"

        self.web_view.setUrl(QUrl(url))

    def update_address_bar(self, url: QUrl) -> None:
        self.address_bar.setText(url.toString())

    def update_window_title(self, title: str) -> None:
        self.setWindowTitle(title or APP_TITLE)

    @staticmethod
    def is_search_query(value: str) -> bool:
        if QUrl(value).scheme():
            return False

        first_word = value.split()[0]
        return (
            " " in value
            or ("." not in first_word and first_word.lower() != "localhost")
        )


def main() -> int:
    app = QApplication(sys.argv)
    window = BrowserWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
