from __future__ import annotations

import base64
import sys
from pathlib import Path
from urllib.parse import quote_plus

from PyQt6.QtCore import QByteArray, QSize, Qt, QUrl
from PyQt6.QtGui import QIcon, QPainter, QPixmap
from PyQt6.QtSvg import QSvgRenderer
from PyQt6.QtWidgets import (
    QApplication,
    QFrame,
    QHBoxLayout,
    QLineEdit,
    QMainWindow,
    QToolBar,
    QToolButton,
    QVBoxLayout,
    QWidget,
)

from PyQt6.QtWebEngineWidgets import QWebEngineView


APP_TITLE = "morphine"
HOME_URL = "morphine://home"
GOOGLE_SEARCH_URL = "https://www.google.com/search?q="

ASSETS_DIR = Path(__file__).resolve().parent
LOGO_PATH = ASSETS_DIR / "morphine_logo.png"

HOME_HTML_TEMPLATE = """
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
      --md-sys-color-surface: #FAF6FF;
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
      align-items: flex-start;
      justify-content: center;
      padding: 12vh 24px 24px;
      background:
        radial-gradient(1200px 600px at 18% -10%, rgba(234, 221, 255, 0.55) 0%, transparent 60%),
        radial-gradient(1000px 500px at 90% 110%, rgba(208, 188, 255, 0.40) 0%, transparent 60%),
        var(--md-sys-color-surface);
    }

    main {
      width: min(680px, 100%);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 28px;
    }

    .brand {
      display: flex;
      align-items: baseline;
      justify-content: center;
      gap: 0;
      margin: 0;
      font-family: "Roboto Flex", "Roboto", sans-serif;
      font-size: clamp(72px, 11vw, 116px);
      line-height: 1;
      font-weight: 600;
      letter-spacing: -0.04em;
    }

    .brand-mark {
      height: 0.6em;
      width: auto;
      display: block;
      object-fit: contain;
      margin: 0 -0.01em 0 -0.04em;
      filter: drop-shadow(0 5px 16px rgba(103, 80, 164, 0.20));
    }

    .brand-text {
      background: linear-gradient(95deg, #C5AAEC 0%, #B29BDB 100%);
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
      -webkit-text-fill-color: transparent;
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
      width: 22px;
      height: 22px;
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
      align-items: center;
    }

    .chip {
      height: 36px;
      padding: 0 16px;
      display: inline-flex;
      align-items: center;
      gap: 8px;
      border: 1px solid var(--md-sys-color-outline-variant);
      border-radius: 999px;
      background: transparent;
      color: var(--md-sys-color-on-surface-variant);
      font: inherit;
      font-size: 14px;
      letter-spacing: 0.1px;
      text-decoration: none;
      cursor: pointer;
      transition: background 150ms ease, color 150ms ease, border-color 150ms ease;
    }

    .chip:hover {
      background: var(--md-sys-color-surface-container);
      color: var(--md-sys-color-on-surface);
      border-color: var(--md-sys-color-outline);
    }

    .chip-icon {
      width: 16px;
      height: 16px;
      color: currentColor;
      flex-shrink: 0;
    }

    .chip.add {
      width: 36px;
      height: 36px;
      padding: 0;
      justify-content: center;
      color: var(--md-sys-color-on-surface-variant);
    }

    .chip.add .chip-icon {
      width: 18px;
      height: 18px;
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
    <h1 class="brand">__LOGO_IMG__<span class="brand-text">orphine</span></h1>

    <form class="search" action="https://www.google.com/search" method="get" role="search">
      <svg class="search-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">
        <path d="M15.5 14h-.79l-.28-.27a6.471 6.471 0 0 0 1.48-5.34C15.27 5.6 12.99 3.4 10.2 3.06A6.502 6.502 0 0 0 3.06 10.2c.34 2.79 2.54 5.07 5.33 5.71a6.471 6.471 0 0 0 5.34-1.48l.27.28v.79l4.25 4.25c.41.41 1.08.41 1.49 0 .41-.41.41-1.08 0-1.49L15.5 14zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/>
      </svg>
      <input name="q" type="search" placeholder="Search the web or type a URL" autofocus autocomplete="off">
      <button class="search-btn" type="submit">Search</button>
    </form>

    <nav class="chips" aria-label="Quick links">
      <a class="chip" href="https://www.google.com">
        <svg class="chip-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M12 2a10 10 0 1 0 10 10A10 10 0 0 0 12 2zm6.86 6h-2.96a15.6 15.6 0 0 0-1.38-3.56A8.04 8.04 0 0 1 18.86 8zM12 4a13.6 13.6 0 0 1 1.74 4H10.26A13.6 13.6 0 0 1 12 4zM4.26 14a8.05 8.05 0 0 1 0-4h3.16a16.7 16.7 0 0 0 0 4zm.88 2h2.96a15.6 15.6 0 0 0 1.38 3.56A8.04 8.04 0 0 1 5.14 16zM8.1 8H5.14a8.04 8.04 0 0 1 4.34-3.56A15.6 15.6 0 0 0 8.1 8zM12 20a13.6 13.6 0 0 1-1.74-4h3.48A13.6 13.6 0 0 1 12 20zm2.16-6H9.84a14.7 14.7 0 0 1 0-4h4.32a14.7 14.7 0 0 1 0 4zm.36 5.56A15.6 15.6 0 0 0 15.9 16h2.96a8.04 8.04 0 0 1-4.34 3.56zM16.58 14a16.7 16.7 0 0 0 0-4h3.16a8.05 8.05 0 0 1 0 4z"/></svg>
        Google
      </a>
      <a class="chip" href="https://github.com">
        <svg class="chip-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M9.4 16.6 4.8 12l4.6-4.6L8 6l-6 6 6 6zm5.2 0L19.2 12l-4.6-4.6L16 6l6 6-6 6z"/></svg>
        GitHub
      </a>
      <a class="chip" href="https://news.ycombinator.com">
        <svg class="chip-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M21 19V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2zM8.9 13.98l2.6-3.34 2 2.4 2.6-3.35L19.5 14H5l3.9-.02z"/></svg>
        Hacker News
      </a>
      <a class="chip" href="https://wikipedia.org">
        <svg class="chip-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M18 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V4a2 2 0 0 0-2-2zM6 4h5v8l-2.5-1.5L6 12V4z"/></svg>
        Wikipedia
      </a>
      <a class="chip add" href="morphine://home" aria-label="Add shortcut">
        <svg class="chip-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M11 5h2v6h6v2h-6v6h-2v-6H5v-2h6V5z"/></svg>
      </a>
    </nav>
  </main>
</body>
</html>
"""


WINDOW_QSS = """
QMainWindow, QWidget#chromeRoot {
    background: #FAF6FF;
}
QToolBar#chromeBar {
    background: #FAF6FF;
    border: 0;
    padding: 8px 12px;
    spacing: 4px;
}
QToolBar#chromeBar::separator { background: transparent; }

QToolButton[chromeNav="true"] {
    background: transparent;
    border: 0;
    border-radius: 18px;
    padding: 0;
    min-width: 36px;
    min-height: 36px;
}
QToolButton[chromeNav="true"]:hover { background: rgba(103, 80, 164, 0.10); }
QToolButton[chromeNav="true"]:pressed { background: rgba(103, 80, 164, 0.20); }
QToolButton[chromeNav="true"]:disabled { color: #C0BAC9; }

QFrame#addressPill {
    background: #ECE6F0;
    border: 0;
    border-radius: 22px;
}
QFrame#addressPill:hover { background: #E5DEEC; }
QFrame#addressPill QLineEdit {
    border: 0;
    background: transparent;
    color: #1D1B20;
    font-size: 14px;
    selection-background-color: #EADDFF;
    selection-color: #21005D;
}
QFrame#addressPill QToolButton#lockBtn {
    background: transparent;
    border: 0;
    min-width: 24px;
    min-height: 24px;
}
QFrame#addressPill QToolButton#starBtn {
    background: transparent;
    border: 0;
    min-width: 28px;
    min-height: 28px;
    border-radius: 14px;
}
QFrame#addressPill QToolButton#starBtn:hover { background: rgba(103, 80, 164, 0.12); }

QToolButton#dotsBtn {
    background: transparent;
    border: 0;
    border-radius: 18px;
    min-width: 36px;
    min-height: 36px;
}
QToolButton#dotsBtn:hover { background: rgba(103, 80, 164, 0.10); }

QToolButton#avatar {
    background: #6750A4;
    color: #FFFFFF;
    border: 0;
    border-radius: 16px;
    font-weight: 700;
    font-size: 13px;
    min-width: 32px;
    min-height: 32px;
    max-width: 32px;
    max-height: 32px;
}
QToolButton#avatar:hover { background: #765FB6; }
"""


PRIMARY_COLOR = "#6750A4"
ON_SURFACE_VARIANT = "#49454F"

ICON_SVGS = {
    "arrow_back": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z"/>'
        "</svg>"
    ),
    "arrow_forward": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M12 4l-1.41 1.41L16.17 11H4v2h12.17l-5.58 5.59L12 20l8-8z"/>'
        "</svg>"
    ),
    "refresh": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M17.65 6.35A7.96 7.96 0 0 0 12 4c-4.42 0-7.99 3.58-7.99 8s3.57 8 7.99 8'
        ' c3.73 0 6.84-2.55 7.73-6h-2.08A5.99 5.99 0 0 1 12 18c-3.31 0-6-2.69-6-6'
        ' s2.69-6 6-6c1.66 0 3.14.69 4.22 1.78L13 11h7V4l-2.35 2.35z"/>'
        "</svg>"
    ),
    "lock": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M18 8h-1V6c0-2.76-2.24-5-5-5S7 3.24 7 6v2H6c-1.1 0-2 .9-2 2v10'
        ' c0 1.1.9 2 2 2h12c1.1 0 2-.9 2-2V10c0-1.1-.9-2-2-2zm-6 9c-1.1 0-2-.9-2-2'
        ' s.9-2 2-2 2 .9 2 2-.9 2-2 2zm3.1-9H8.9V6c0-1.71 1.39-3.1 3.1-3.1'
        ' 1.71 0 3.1 1.39 3.1 3.1v2z"/>'
        "</svg>"
    ),
    "star_outline": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M22 9.24l-7.19-.62L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21 12 17.27'
        ' 18.18 21l-1.63-7.03L22 9.24zM12 15.4l-3.76 2.27 1-4.28-3.32-2.88 4.38-.38'
        ' L12 6.1l1.71 4.04 4.38.38-3.32 2.88 1 4.28L12 15.4z"/>'
        "</svg>"
    ),
    "star_filled": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M12 17.27L18.18 21l-1.64-7.03L22 9.24l-7.19-.61L12 2 9.19 8.63'
        ' 2 9.24l5.46 4.73L5.82 21 12 17.27z"/>'
        "</svg>"
    ),
    "more_vert": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M12 8c1.1 0 2-.9 2-2s-.9-2-2-2-2 .9-2 2 .9 2 2 2zm0 2c-1.1 0-2 .9-2 2'
        ' s.9 2 2 2 2-.9 2-2-.9-2-2-2zm0 6c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2z"/>'
        "</svg>"
    ),
}


def svg_icon(name: str, color: str = PRIMARY_COLOR, size: int = 24) -> QIcon:
    svg = ICON_SVGS[name].format(c=color)
    renderer = QSvgRenderer(QByteArray(svg.encode("utf-8")))
    pixmap = QPixmap(size, size)
    pixmap.fill(Qt.GlobalColor.transparent)
    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
    painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform, True)
    renderer.render(painter)
    painter.end()
    return QIcon(pixmap)


def _logo_data_uri() -> str:
    if not LOGO_PATH.is_file():
        return ""
    b64 = base64.b64encode(LOGO_PATH.read_bytes()).decode("ascii")
    return f"data:image/png;base64,{b64}"


def render_home_html() -> str:
    data_uri = _logo_data_uri()
    if data_uri:
        logo_img = (
            f'<img class="brand-mark" src="{data_uri}" alt="m" draggable="false">'
        )
    else:
        logo_img = (
            '<svg class="brand-mark" xmlns="http://www.w3.org/2000/svg" '
            'viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">'
            '<path d="M3 4h3l3 9 3-9h3v16h-3v-9l-3 9h-2L4 11v9H3V4z"/></svg>'
        )
    return HOME_HTML_TEMPLATE.replace("__LOGO_IMG__", logo_img)


def _make_nav_button(icon_name: str, tooltip: str) -> QToolButton:
    btn = QToolButton()
    btn.setIcon(svg_icon(icon_name))
    btn.setIconSize(QSize(22, 22))
    btn.setToolTip(tooltip)
    btn.setProperty("chromeNav", True)
    btn.setCursor(Qt.CursorShape.PointingHandCursor)
    btn.setAutoRaise(True)
    btn.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonIconOnly)
    return btn


class BrowserWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()

        self.setWindowTitle(APP_TITLE)
        self.resize(1200, 800)
        self.setStyleSheet(WINDOW_QSS)

        self.web_view = QWebEngineView()
        self._home_html = render_home_html()

        self._build_chrome()
        self._build_central()

        self.web_view.urlChanged.connect(self.update_address_bar)
        self.web_view.titleChanged.connect(self.update_window_title)

        self.load_home()

    # ------------------------------------------------------------------ chrome
    def _build_chrome(self) -> None:
        self.back_btn = _make_nav_button("arrow_back", "Back")
        self.back_btn.clicked.connect(self.web_view.back)

        self.forward_btn = _make_nav_button("arrow_forward", "Forward")
        self.forward_btn.clicked.connect(self.web_view.forward)

        self.reload_btn = _make_nav_button("refresh", "Reload")
        self.reload_btn.clicked.connect(self.web_view.reload)

        address_pill = self._build_address_pill()

        self.dots_btn = QToolButton()
        self.dots_btn.setObjectName("dotsBtn")
        self.dots_btn.setIcon(svg_icon("more_vert"))
        self.dots_btn.setIconSize(QSize(22, 22))
        self.dots_btn.setToolTip("Menu")
        self.dots_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.dots_btn.setAutoRaise(True)
        self.dots_btn.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonIconOnly)

        self.avatar_btn = QToolButton()
        self.avatar_btn.setObjectName("avatar")
        self.avatar_btn.setText("U")
        self.avatar_btn.setToolTip("Profile")
        self.avatar_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.avatar_btn.setAutoRaise(True)

        toolbar = QToolBar("Navigation")
        toolbar.setObjectName("chromeBar")
        toolbar.setMovable(False)
        toolbar.setFloatable(False)

        bar = QWidget()
        layout = QHBoxLayout(bar)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(4)
        layout.addWidget(self.back_btn)
        layout.addWidget(self.forward_btn)
        layout.addWidget(self.reload_btn)
        layout.addSpacing(8)
        layout.addWidget(address_pill, stretch=1)
        layout.addSpacing(8)
        layout.addWidget(self.dots_btn)
        layout.addWidget(self.avatar_btn)

        toolbar.addWidget(bar)
        self.addToolBar(toolbar)

    def _build_address_pill(self) -> QFrame:
        pill = QFrame()
        pill.setObjectName("addressPill")
        pill.setFixedHeight(40)

        self._lock_icon_outline = svg_icon("lock", color=ON_SURFACE_VARIANT, size=18)
        lock_btn = QToolButton()
        lock_btn.setObjectName("lockBtn")
        lock_btn.setIcon(self._lock_icon_outline)
        lock_btn.setIconSize(QSize(16, 16))
        lock_btn.setToolTip("Connection is private")
        lock_btn.setAutoRaise(True)
        lock_btn.setEnabled(False)
        lock_btn.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonIconOnly)

        self.address_bar = QLineEdit()
        self.address_bar.setFrame(False)
        self.address_bar.setClearButtonEnabled(False)
        self.address_bar.setPlaceholderText("Search the web or type a URL")
        self.address_bar.returnPressed.connect(self.open_address)

        self._star_outline = svg_icon("star_outline")
        self._star_filled = svg_icon("star_filled")
        self.star_btn = QToolButton()
        self.star_btn.setObjectName("starBtn")
        self.star_btn.setIcon(self._star_outline)
        self.star_btn.setIconSize(QSize(20, 20))
        self.star_btn.setToolTip("Bookmark this page")
        self.star_btn.setCheckable(True)
        self.star_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.star_btn.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonIconOnly)
        self.star_btn.toggled.connect(self._on_star_toggled)

        layout = QHBoxLayout(pill)
        layout.setContentsMargins(8, 0, 4, 0)
        layout.setSpacing(6)
        layout.addWidget(lock_btn)
        layout.addWidget(self.address_bar, stretch=1)
        layout.addWidget(self.star_btn)
        return pill

    def _on_star_toggled(self, checked: bool) -> None:
        self.star_btn.setIcon(self._star_filled if checked else self._star_outline)

    def _build_central(self) -> None:
        page_container = QWidget()
        page_container.setObjectName("chromeRoot")
        layout = QVBoxLayout(page_container)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.web_view)
        self.setCentralWidget(page_container)

    # --------------------------------------------------------------- behaviour
    def open_address(self) -> None:
        self.load_url(self.address_bar.text())

    def load_home(self) -> None:
        self.web_view.setHtml(self._home_html, QUrl(HOME_URL))
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
        self.star_btn.setChecked(False)

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
    app.setApplicationName(APP_TITLE)
    window = BrowserWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
