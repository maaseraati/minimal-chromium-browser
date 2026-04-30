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
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Roboto+Flex:opsz,wght@8..144,400;8..144,500;8..144,700&family=Roboto:wght@400;500;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --md-sys-color-primary: #6750A4;
      --md-sys-color-on-primary: #FFFFFF;
      --md-sys-color-primary-hover: #765FB6;
      --md-sys-color-primary-pressed: #5A4593;
      --md-sys-color-primary-container: #EADDFF;
      --md-sys-color-on-primary-container: #21005D;
      --md-sys-color-surface: #FEF7FF;
      --md-sys-color-on-surface: #1D1B20;
      --md-sys-color-on-surface-variant: #49454F;
      --md-sys-color-surface-container: #F3EDF7;
      --md-sys-color-surface-container-high: #ECE6F0;
      --md-sys-color-outline: #79747E;
      --md-sys-color-outline-variant: #CAC4D0;
      --md-sys-elevation-1: 0 1px 2px rgba(0, 0, 0, 0.30), 0 1px 3px 1px rgba(0, 0, 0, 0.15);
      --md-sys-elevation-2: 0 1px 2px rgba(0, 0, 0, 0.30), 0 2px 6px 2px rgba(0, 0, 0, 0.15);

      color-scheme: light;
      font-family: "Roboto Flex", "Roboto", system-ui, -apple-system, "Segoe UI", Helvetica, Arial, sans-serif;
      background: var(--md-sys-color-surface);
      color: var(--md-sys-color-on-surface);
    }

    *, *::before, *::after { box-sizing: border-box; }

    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 24px;
      background:
        radial-gradient(1200px 600px at 18% -10%, rgba(234, 221, 255, 0.55) 0%, transparent 60%),
        radial-gradient(1000px 500px at 90% 110%, rgba(208, 188, 255, 0.40) 0%, transparent 60%),
        var(--md-sys-color-surface);
    }

    main {
      width: min(640px, 100%);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 32px;
    }

    .brand {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 12px;
    }

    .brand-mark {
      width: 64px;
      height: 64px;
      border-radius: 20px;
      background: linear-gradient(135deg, #6750A4 0%, #9A82DB 100%);
      display: grid;
      place-items: center;
      color: #FFFFFF;
      box-shadow: var(--md-sys-elevation-1);
    }

    h1 {
      margin: 0;
      font-family: "Roboto Flex", "Roboto", sans-serif;
      font-size: clamp(48px, 8vw, 72px);
      line-height: 1.05;
      font-weight: 500;
      letter-spacing: -0.02em;
      color: var(--md-sys-color-on-surface);
    }

    p.tagline {
      margin: 0;
      font-size: 14px;
      letter-spacing: 0.01em;
      color: var(--md-sys-color-on-surface-variant);
    }

    form.search {
      width: 100%;
      display: flex;
      align-items: center;
      gap: 8px;
      padding: 4px 4px 4px 16px;
      background: var(--md-sys-color-surface-container-high);
      border-radius: 28px;
      box-shadow: 0 1px 2px rgba(0, 0, 0, 0.06);
      transition: box-shadow 150ms ease, background 150ms ease;
    }

    form.search:hover {
      background: var(--md-sys-color-surface-container);
    }

    form.search:focus-within {
      background: var(--md-sys-color-surface-container);
      box-shadow: var(--md-sys-elevation-2);
    }

    .search-icon {
      width: 24px;
      height: 24px;
      color: var(--md-sys-color-on-surface-variant);
      flex-shrink: 0;
    }

    input[name="q"] {
      flex: 1;
      min-width: 0;
      height: 48px;
      border: 0;
      background: transparent;
      outline: none;
      font: inherit;
      font-size: 16px;
      color: var(--md-sys-color-on-surface);
      caret-color: var(--md-sys-color-primary);
    }

    input[name="q"]::placeholder {
      color: var(--md-sys-color-on-surface-variant);
    }

    button.search-btn {
      height: 48px;
      padding: 0 24px;
      border: 0;
      border-radius: 24px;
      background: var(--md-sys-color-primary);
      color: var(--md-sys-color-on-primary);
      font-family: inherit;
      font-size: 14px;
      font-weight: 500;
      letter-spacing: 0.1px;
      cursor: pointer;
      transition: background 150ms ease, box-shadow 150ms ease;
    }

    button.search-btn:hover {
      background: var(--md-sys-color-primary-hover);
      box-shadow: var(--md-sys-elevation-1);
    }

    button.search-btn:active {
      background: var(--md-sys-color-primary-pressed);
    }

    .chips {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      justify-content: center;
    }

    .chip {
      height: 32px;
      padding: 0 16px;
      display: inline-flex;
      align-items: center;
      gap: 8px;
      border: 1px solid var(--md-sys-color-outline-variant);
      border-radius: 8px;
      background: transparent;
      color: var(--md-sys-color-on-surface-variant);
      font: inherit;
      font-size: 14px;
      letter-spacing: 0.1px;
      text-decoration: none;
      cursor: pointer;
      transition: background 150ms ease, color 150ms ease;
    }

    .chip:hover {
      background: var(--md-sys-color-surface-container);
      color: var(--md-sys-color-on-surface);
    }

    @media (prefers-color-scheme: dark) {
      :root {
        --md-sys-color-primary: #D0BCFF;
        --md-sys-color-on-primary: #381E72;
        --md-sys-color-primary-hover: #C0ADEC;
        --md-sys-color-primary-pressed: #B49BE0;
        --md-sys-color-primary-container: #4F378B;
        --md-sys-color-on-primary-container: #EADDFF;
        --md-sys-color-surface: #141218;
        --md-sys-color-on-surface: #E6E0E9;
        --md-sys-color-on-surface-variant: #CAC4D0;
        --md-sys-color-surface-container: #211F26;
        --md-sys-color-surface-container-high: #2B2930;
        --md-sys-color-outline: #938F99;
        --md-sys-color-outline-variant: #49454F;
      }
    }
  </style>
</head>
<body>
  <main>
    <div class="brand">
      <div class="brand-mark" aria-hidden="true">
        <svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24" fill="currentColor">
          <path d="M12 2a10 10 0 1 0 0 20 10 10 0 0 0 0-20zm0 4a3.5 3.5 0 0 1 3.5 3.5c0 1.93-3.5 5-3.5 5s-3.5-3.07-3.5-5A3.5 3.5 0 0 1 12 6zm0 12.5c-2.5 0-4.71-1.28-6-3.22.03-1.99 4-3.08 6-3.08 1.99 0 5.97 1.09 6 3.08-1.29 1.94-3.5 3.22-6 3.22z"/>
        </svg>
      </div>
      <h1>morphine</h1>
      <p class="tagline">Search the web with a Material You feel</p>
    </div>

    <form class="search" action="https://www.google.com/search" method="get" role="search">
      <svg class="search-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">
        <path d="M15.5 14h-.79l-.28-.27a6.471 6.471 0 0 0 1.48-5.34C15.27 5.6 12.99 3.4 10.2 3.06A6.502 6.502 0 0 0 3.06 10.2c.34 2.79 2.54 5.07 5.33 5.71a6.471 6.471 0 0 0 5.34-1.48l.27.28v.79l4.25 4.25c.41.41 1.08.41 1.49 0 .41-.41.41-1.08 0-1.49L15.5 14zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/>
      </svg>
      <input name="q" type="search" placeholder="Search Google or type a URL" autofocus autocomplete="off">
      <button class="search-btn" type="submit">Search</button>
    </form>

    <nav class="chips" aria-label="Quick links">
      <a class="chip" href="https://www.google.com">Google</a>
      <a class="chip" href="https://github.com">GitHub</a>
      <a class="chip" href="https://news.ycombinator.com">Hacker News</a>
      <a class="chip" href="https://wikipedia.org">Wikipedia</a>
    </nav>
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
