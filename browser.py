from __future__ import annotations

import base64
import sys
from dataclasses import dataclass
from html import escape
from pathlib import Path
from urllib.parse import quote_plus

from PyQt6.QtCore import (
    QByteArray,
    QEasingCurve,
    QEvent,
    QParallelAnimationGroup,
    QPoint,
    QPropertyAnimation,
    QRect,
    QSize,
    Qt,
    QUrl,
)
from PyQt6.QtGui import QIcon, QMouseEvent, QPainter, QPixmap
from PyQt6.QtSvg import QSvgRenderer
from PyQt6.QtWidgets import (
    QApplication,
    QFrame,
    QGraphicsOpacityEffect,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QSizePolicy,
    QStackedWidget,
    QToolButton,
    QVBoxLayout,
    QWidget,
)

from PyQt6.QtWebEngineWidgets import QWebEngineView


APP_TITLE = "morphine"
HOME_URL = "morphine://home"
HISTORY_URL = "morphine://history"
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
      transition: background 150ms ease;
    }

    form.search:hover {
      background: var(--md-sys-color-surface-container);
    }

    form.search:focus-within {
      background: var(--md-sys-color-surface-container);
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
      font-weight: 700;
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

HISTORY_HTML_TEMPLATE = """
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>History</title>
  <style>
    :root {
      color-scheme: light;
      font-family: "Roboto", system-ui, -apple-system, "Segoe UI", Helvetica, Arial, sans-serif;
      background: #FAF6FF;
      color: #1D1B20;
    }

    body {
      margin: 0;
      min-height: 100vh;
      padding: 64px 24px;
      background:
        radial-gradient(900px 420px at 20% -10%, rgba(234, 221, 255, 0.50) 0%, transparent 60%),
        #FAF6FF;
    }

    main {
      width: min(820px, 100%);
      margin: 0 auto;
    }

    h1 {
      margin: 0 0 24px;
      font-size: 42px;
      line-height: 1.1;
      color: #6750A4;
    }

    .empty {
      padding: 28px;
      border-radius: 28px;
      background: #ECE6F0;
      color: #49454F;
      font-size: 16px;
    }

    ol {
      list-style: none;
      margin: 0;
      padding: 0;
      display: grid;
      gap: 10px;
    }

    a {
      display: block;
      padding: 16px 18px;
      border-radius: 22px;
      background: #F3EDF7;
      color: #1D1B20;
      text-decoration: none;
    }

    a:hover {
      background: #ECE6F0;
    }

    strong {
      display: block;
      font-size: 16px;
      margin-bottom: 4px;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }

    span {
      display: block;
      color: #49454F;
      font-size: 13px;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
  </style>
</head>
<body>
  <main>
    <h1>History</h1>
    __HISTORY_ITEMS__
  </main>
</body>
</html>
"""


WINDOW_QSS = """
QMainWindow, QWidget#chromeRoot {
    background: #FAF6FF;
}
QWidget#chromeBar {
    background: #FAF6FF;
    border: 0;
}
QWidget#navGroup {
    background: #F3EDF7;
    border: 0;
    border-radius: 30px;
}

QToolButton[chromeNav="true"] {
    background: transparent;
    border: 0;
    border-radius: 22px;
    padding: 0;
    min-width: 44px;
    min-height: 44px;
}
QToolButton[chromeNav="true"]:hover { background: rgba(103, 80, 164, 0.10); }
QToolButton[chromeNav="true"]:pressed { background: rgba(103, 80, 164, 0.20); }
QToolButton[chromeNav="true"]:disabled { color: #C0BAC9; }

QFrame#addressPill {
    background: #F3EDF7;
    border: 0;
    border-radius: 30px;
}
QFrame#addressPill:hover { background: #E5DEEC; }
QFrame#addressPill QLineEdit {
    border: 0;
    background: transparent;
    color: #1D1B20;
    font-size: 20px;
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
    border-radius: 22px;
    min-width: 44px;
    min-height: 44px;
}
QToolButton#dotsBtn:hover { background: rgba(103, 80, 164, 0.10); }

QToolButton#avatar {
    background: #6750A4;
    color: #FFFFFF;
    border: 0;
    border-radius: 20px;
    font-weight: 700;
    font-size: 20px;
    min-width: 40px;
    min-height: 40px;
    max-width: 40px;
    max-height: 40px;
}
QToolButton#avatar:hover { background: #765FB6; }

QWidget#appShell {
    background: #FAF6FF;
    border: 0;
    border-radius: 0;
}
QWidget#tabStrip {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FCF8FF, stop:1 #FEFBFF);
    border: 0;
}
QWidget#browserTab {
    background: #FFFFFF;
    border: 0;
    border-radius: 8px;
    color: #1D1B20;
}
QWidget#browserTab[active="true"] {
    background: #F5EEFF;
}
QLabel#tabBadge {
    background: #6750A4;
    color: #FFFFFF;
    border-radius: 14px;
    font-weight: 700;
    font-size: 16px;
}
QLabel#tabTitle {
    color: #1D1B20;
    font-size: 16px;
}
QToolButton#tabCloseBtn {
    background: transparent;
    border: 0;
    border-radius: 13px;
    min-width: 26px;
    min-height: 26px;
}
QToolButton#tabCloseBtn:hover {
    background: rgba(29, 27, 32, 0.08);
}
QToolButton#newTabBtn {
    background: #F1EAF8;
    border: 0;
    border-radius: 7px;
    min-width: 48px;
    min-height: 34px;
    max-width: 48px;
    max-height: 34px;
}
QToolButton#newTabBtn:hover { background: #EADDFF; }
QStackedWidget#pages {
    background: #FAF6FF;
    border: 0;
}
QWidget#windowControls {
    background: transparent;
}
QToolButton[windowControl="true"] {
    background: transparent;
    border: 0;
    border-radius: 14px;
    min-width: 42px;
    min-height: 32px;
    font-size: 24px;
    color: #1D1B20;
}
QToolButton[windowControl="true"]:hover {
    background: rgba(29, 27, 32, 0.08);
}
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
    "history": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M13 3a9 9 0 1 1-8.95 8H2l3-3.01L8 11H6.06A7 7 0 1 0 13 5'
        'a6.97 6.97 0 0 0-4.95 2.05L6.64 5.64A8.95 8.95 0 0 1 13 3zm-1 4h1.5'
        'v5.25l4.5 2.67-.75 1.23L12 13V7z"/>'
        "</svg>"
    ),
    "home": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z"/>'
        "</svg>"
    ),
    "close": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M18.3 5.71 12 12l6.3 6.29-1.41 1.41L10.59 13.41 4.29 19.7'
        ' 2.88 18.29 9.17 12 2.88 5.71 4.29 4.3l6.3 6.29 6.29-6.29z"/>'
        "</svg>"
    ),
    "add": (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="{c}">'
        '<path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>'
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


class BrowserTab(QWebEngineView):
    def __init__(self, window: BrowserWindow) -> None:
        super().__init__()
        self.window = window

    def createWindow(self, _type: QWebEngineView.WebWindowType) -> QWebEngineView:
        return self.window.add_tab(switch_to=True)


class TabStrip(QWidget):
    """Tab bar that doubles as the window's drag handle.

    Holding LMB on empty space (anywhere not over an interactive child) starts
    a system move so the user can drag the frameless window. Double-clicking
    the same area toggles maximize/restore, matching the standard Windows
    titlebar behaviour.
    """

    def __init__(self, browser_window: BrowserWindow) -> None:
        super().__init__()
        self._browser_window = browser_window
        self._press_pos: QPoint | None = None
        self._press_global: QPoint | None = None
        self._dragging = False

    def _is_drag_target(self, pos: QPoint) -> bool:
        child = self.childAt(pos)
        widget: QWidget | None = child
        while widget is not None and widget is not self:
            if isinstance(widget, (TabButton, QToolButton, QLineEdit)):
                return False
            widget = widget.parentWidget()
        return True

    def mousePressEvent(self, event: QMouseEvent) -> None:
        if event.button() == Qt.MouseButton.LeftButton and self._is_drag_target(
            event.position().toPoint()
        ):
            self._press_pos = event.position().toPoint()
            self._press_global = event.globalPosition().toPoint()
            self._dragging = False
            event.accept()
            return
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        if (
            self._press_pos is not None
            and event.buttons() & Qt.MouseButton.LeftButton
            and not self._dragging
        ):
            delta = event.globalPosition().toPoint() - self._press_global
            if delta.manhattanLength() >= QApplication.startDragDistance():
                self._dragging = True
                self._begin_window_move()
                event.accept()
                return
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event: QMouseEvent) -> None:
        self._press_pos = None
        self._press_global = None
        self._dragging = False
        super().mouseReleaseEvent(event)

    def mouseDoubleClickEvent(self, event: QMouseEvent) -> None:
        if event.button() == Qt.MouseButton.LeftButton and self._is_drag_target(
            event.position().toPoint()
        ):
            self._browser_window.toggle_window_maximized()
            event.accept()
            return
        super().mouseDoubleClickEvent(event)

    def _begin_window_move(self) -> None:
        window = self._browser_window
        if window.isMaximized():
            window.showNormal()
        handle = window.windowHandle()
        if handle is not None:
            handle.startSystemMove()


class TabButton(QWidget):
    DEFAULT_WIDTH = 260
    DEFAULT_HEIGHT = 48

    def __init__(
        self,
        index: int,
        title: str,
        window: BrowserWindow,
    ) -> None:
        super().__init__()
        self.index = index
        self.window = window
        self.is_closing = False
        self.setObjectName("browserTab")
        self.setFixedHeight(self.DEFAULT_HEIGHT)
        self.setMinimumWidth(0)
        self.setMaximumWidth(self.DEFAULT_WIDTH)
        self.setSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        self.setCursor(Qt.CursorShape.PointingHandCursor)

        layout = QHBoxLayout(self)
        layout.setContentsMargins(16, 0, 12, 0)
        layout.setSpacing(12)

        self.icon_label = QLabel()
        self.icon_label.setObjectName("tabBadge")
        self.icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.icon_label.setFixedSize(28, 28)
        layout.addWidget(self.icon_label)

        self.title_label = QLabel(title)
        self.title_label.setObjectName("tabTitle")
        self.title_label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        layout.addWidget(self.title_label)

        self.close_btn = QToolButton()
        self.close_btn.setObjectName("tabCloseBtn")
        self.close_btn.setIcon(svg_icon("close", color=ON_SURFACE_VARIANT, size=18))
        self.close_btn.setIconSize(QSize(16, 16))
        self.close_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.close_btn.setAutoRaise(True)
        self.close_btn.clicked.connect(self._on_close_clicked)
        layout.addWidget(self.close_btn)
        self.set_title(title)

    def _on_close_clicked(self) -> None:
        if self.is_closing:
            return
        self.window.close_tab(self.index)

    def mousePressEvent(self, event) -> None:
        if self.is_closing:
            super().mousePressEvent(event)
            return
        if event.button() == Qt.MouseButton.LeftButton:
            self.window.select_tab(self.index)
        super().mousePressEvent(event)

    def set_active(self, active: bool) -> None:
        self.setProperty("active", active)
        self.style().unpolish(self)
        self.style().polish(self)

    def set_title(self, title: str) -> None:
        self.title_label.setText(title)
        if title.lower() == "history":
            self.icon_label.setText("")
            self.icon_label.setStyleSheet("background: transparent;")
            self.icon_label.setPixmap(svg_icon("history").pixmap(28, 28))
        else:
            self.icon_label.setPixmap(QPixmap())
            self.icon_label.setStyleSheet("")
            self.icon_label.setText("m")


@dataclass
class HistoryEntry:
    title: str
    url: str


class BrowserWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()

        self.setWindowFlag(Qt.WindowType.FramelessWindowHint, True)
        self.setWindowTitle(APP_TITLE)
        self.resize(1200, 800)
        self.setStyleSheet(WINDOW_QSS)

        self._home_html = render_home_html()
        self._history: list[HistoryEntry] = []
        self._recording_history = True
        self._tab_animations: list[QParallelAnimationGroup] = []
        self._window_anim: QParallelAnimationGroup | None = None
        self._normal_geometry: QRect | None = None
        self._is_minimizing = False
        self._suppress_state_anim = False
        self.current_tab_index = -1

        self._build_chrome()
        self._build_central()

        self.add_tab(switch_to=True)

    def toggle_window_maximized(self) -> None:
        if self.isMaximized():
            self.showNormal()
        else:
            self.showMaximized()

    # ------------------------------------------------------------------ chrome
    def _build_chrome(self) -> None:
        self.back_btn = _make_nav_button("arrow_back", "Back")
        self.back_btn.clicked.connect(lambda: self.active_web_view().back())

        self.forward_btn = _make_nav_button("arrow_forward", "Forward")
        self.forward_btn.clicked.connect(lambda: self.active_web_view().forward())

        self.reload_btn = _make_nav_button("refresh", "Reload")
        self.reload_btn.clicked.connect(lambda: self.active_web_view().reload())

        self.home_btn = _make_nav_button("home", "Home")
        self.home_btn.clicked.connect(lambda: self.load_home())

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

        bar = QWidget()
        bar.setObjectName("chromeBar")
        bar.setFixedHeight(92)
        bar.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        layout = QHBoxLayout(bar)
        layout.setContentsMargins(14, 14, 22, 22)
        layout.setSpacing(20)

        nav_group = QWidget()
        nav_group.setObjectName("navGroup")
        nav_group.setFixedHeight(60)
        nav_layout = QHBoxLayout(nav_group)
        nav_layout.setContentsMargins(14, 0, 14, 0)
        nav_layout.setSpacing(10)
        nav_layout.addWidget(self.back_btn)
        nav_layout.addWidget(self.forward_btn)
        nav_layout.addWidget(self.reload_btn)
        nav_layout.addWidget(self.home_btn)
        layout.addWidget(nav_group)
        layout.addWidget(address_pill)
        layout.addStretch()
        layout.addWidget(self.dots_btn)
        self.dots_btn.clicked.connect(self.show_history)
        layout.addSpacing(18)
        layout.addWidget(self.avatar_btn)
        self.chrome_bar = bar

    def _build_address_pill(self) -> QFrame:
        pill = QFrame()
        pill.setObjectName("addressPill")
        pill.setFixedHeight(60)
        pill.setMinimumWidth(680)
        pill.setMaximumWidth(680)

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
        layout.setContentsMargins(14, 0, 8, 0)
        layout.setSpacing(8)
        layout.addWidget(lock_btn)
        layout.addWidget(self.address_bar, stretch=1)
        layout.addWidget(self.star_btn)
        return pill

    def _on_star_toggled(self, checked: bool) -> None:
        self.star_btn.setIcon(self._star_filled if checked else self._star_outline)

    def _build_central(self) -> None:
        page_container = QWidget()
        page_container.setObjectName("chromeRoot")
        outer_layout = QVBoxLayout(page_container)
        outer_layout.setContentsMargins(0, 0, 0, 0)
        outer_layout.setSpacing(0)

        shell = QWidget()
        shell.setObjectName("appShell")
        layout = QVBoxLayout(shell)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        tab_strip = TabStrip(self)
        tab_strip.setObjectName("tabStrip")
        tab_strip.setFixedHeight(68)
        tab_layout = QHBoxLayout(tab_strip)
        tab_layout.setContentsMargins(14, 14, 24, 0)
        tab_layout.setSpacing(12)

        self.tab_buttons_container = QWidget()
        self.tab_buttons_layout = QHBoxLayout(self.tab_buttons_container)
        self.tab_buttons_layout.setContentsMargins(0, 0, 0, 0)
        self.tab_buttons_layout.setSpacing(12)
        tab_layout.addWidget(self.tab_buttons_container)

        self.new_tab_btn = QToolButton()
        self.new_tab_btn.setObjectName("newTabBtn")
        self.new_tab_btn.setIcon(svg_icon("add"))
        self.new_tab_btn.setIconSize(QSize(20, 20))
        self.new_tab_btn.setToolTip("New tab")
        self.new_tab_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.new_tab_btn.setAutoRaise(True)
        self.new_tab_btn.clicked.connect(lambda: self.add_tab(switch_to=True))
        tab_layout.addWidget(self.new_tab_btn)
        tab_layout.addStretch()

        window_controls = QWidget()
        window_controls.setObjectName("windowControls")
        window_layout = QHBoxLayout(window_controls)
        window_layout.setContentsMargins(0, 0, 0, 0)
        window_layout.setSpacing(18)
        for text, slot in (
            ("–", self.minimize_with_animation),
            ("▢", self.toggle_window_maximized),
            ("×", self.close),
        ):
            btn = QToolButton()
            btn.setText(text)
            btn.setProperty("windowControl", True)
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            btn.clicked.connect(slot)
            window_layout.addWidget(btn)
        tab_layout.addWidget(window_controls)

        self.pages = QStackedWidget()
        self.pages.setObjectName("pages")

        layout.addWidget(tab_strip)
        layout.addWidget(self.chrome_bar)
        layout.addWidget(self.pages)
        outer_layout.addWidget(shell)
        self.setCentralWidget(page_container)

    # --------------------------------------------------------------- behaviour
    def open_address(self) -> None:
        self.load_url(self.address_bar.text())

    def add_tab(self, switch_to: bool = False) -> BrowserTab:
        view = BrowserTab(self)
        view.urlChanged.connect(lambda url, tab=view: self.update_address_bar(tab, url))
        view.titleChanged.connect(lambda title, tab=view: self.update_tab_title(tab, title))
        index = self.pages.addWidget(view)
        tab_button = TabButton(index, APP_TITLE, self)
        self.tab_buttons_layout.addWidget(tab_button)
        self._animate_tab_open(tab_button)
        self.load_home(view)
        self._sync_tab_buttons()
        if switch_to:
            self.select_tab(index)
        return view

    def _animate_tab_open(self, tab_button: TabButton) -> None:
        target_width = tab_button.maximumWidth() or TabButton.DEFAULT_WIDTH
        tab_button.setMaximumWidth(0)

        opacity = QGraphicsOpacityEffect(tab_button)
        opacity.setOpacity(0.0)
        tab_button.setGraphicsEffect(opacity)

        grow = QPropertyAnimation(tab_button, b"maximumWidth", self)
        grow.setDuration(260)
        grow.setStartValue(0)
        grow.setEndValue(target_width)
        grow.setEasingCurve(QEasingCurve.Type.OutCubic)

        fade = QPropertyAnimation(opacity, b"opacity", self)
        fade.setDuration(220)
        fade.setStartValue(0.0)
        fade.setEndValue(1.0)
        fade.setEasingCurve(QEasingCurve.Type.OutCubic)

        group = QParallelAnimationGroup(self)
        group.addAnimation(grow)
        group.addAnimation(fade)
        self._tab_animations.append(group)

        def _finalize() -> None:
            if group in self._tab_animations:
                self._tab_animations.remove(group)
            tab_button.setGraphicsEffect(None)
            tab_button.setMaximumWidth(target_width)

        group.finished.connect(_finalize)
        group.start()

    def close_tab(self, index: int) -> None:
        if self.pages.count() == 1:
            self.load_home(self.active_web_view())
            return

        view = self.pages.widget(index)
        tab_button = self.tab_buttons_layout.itemAt(index).widget()
        if not isinstance(tab_button, TabButton):
            self._remove_tab(tab_button, view)
            return
        if tab_button.is_closing:
            return
        tab_button.is_closing = True
        tab_button.close_btn.setEnabled(False)

        if index == self.current_tab_index and self.pages.count() > 1:
            next_index = index - 1 if index == self.pages.count() - 1 else index + 1
            self.select_tab(next_index)

        opacity = QGraphicsOpacityEffect(tab_button)
        tab_button.setGraphicsEffect(opacity)
        start_width = max(tab_button.width(), 1)
        tab_button.setMinimumWidth(0)
        tab_button.setMaximumWidth(start_width)

        fade = QPropertyAnimation(opacity, b"opacity", self)
        fade.setDuration(220)
        fade.setStartValue(1.0)
        fade.setEndValue(0.0)
        fade.setEasingCurve(QEasingCurve.Type.InOutCubic)

        shrink = QPropertyAnimation(tab_button, b"maximumWidth", self)
        shrink.setDuration(260)
        shrink.setStartValue(start_width)
        shrink.setEndValue(0)
        shrink.setEasingCurve(QEasingCurve.Type.InOutCubic)

        group = QParallelAnimationGroup(self)
        group.addAnimation(fade)
        group.addAnimation(shrink)
        self._tab_animations.append(group)
        group.finished.connect(
            lambda btn=tab_button, v=view: self._finish_close_animation(group, btn, v)
        )
        group.start()

    def _finish_close_animation(
        self,
        animation: QParallelAnimationGroup,
        tab_button: TabButton,
        view: QWidget,
    ) -> None:
        if animation in self._tab_animations:
            self._tab_animations.remove(animation)
        self._remove_tab(tab_button, view)

    def _remove_tab(self, tab_button: QWidget, view: QWidget) -> None:
        button_index = self.tab_buttons_layout.indexOf(tab_button)
        if button_index >= 0:
            item = self.tab_buttons_layout.takeAt(button_index)
            if item is not None:
                widget = item.widget()
                if widget is not None:
                    widget.setParent(None)
                    widget.deleteLater()
        page_index = self.pages.indexOf(view)
        if page_index >= 0:
            self.pages.removeWidget(view)
        view.deleteLater()
        new_count = self.pages.count()
        if self.current_tab_index >= new_count:
            self.current_tab_index = new_count - 1
        if new_count > 0:
            self.select_tab(max(0, self.current_tab_index))
        self._sync_tab_buttons()

    def select_tab(self, index: int) -> None:
        if index < 0 or index >= self.pages.count():
            return
        self.current_tab_index = index
        self.pages.setCurrentIndex(index)
        self._on_current_tab_changed(index)
        self._sync_tab_buttons()

    def _sync_tab_buttons(self) -> None:
        for index in range(self.tab_buttons_layout.count()):
            widget = self.tab_buttons_layout.itemAt(index).widget()
            if isinstance(widget, TabButton):
                widget.index = index
                widget.set_active(index == self.current_tab_index)

    def active_web_view(self) -> BrowserTab:
        tab = self.pages.currentWidget()
        if not isinstance(tab, BrowserTab):
            return self.add_tab(switch_to=True)
        return tab

    def load_home(self, view: BrowserTab | None = None) -> None:
        target = view or self.active_web_view()
        self._recording_history = False
        target.setHtml(self._home_html, QUrl(HOME_URL))
        self._recording_history = True
        if target is self.active_web_view():
            self.address_bar.setText("")

    def show_history(self) -> None:
        self._recording_history = False
        self.active_web_view().setHtml(self.render_history_html(), QUrl(HISTORY_URL))
        self._recording_history = True
        self.address_bar.setText(HISTORY_URL)

    def render_history_html(self) -> str:
        if not self._history:
            items = '<p class="empty">No browsing history yet.</p>'
        else:
            rows = []
            for entry in reversed(self._history[-50:]):
                title = escape(entry.title or entry.url)
                url = escape(entry.url, quote=True)
                rows.append(
                    f'<li><a href="{url}"><strong>{title}</strong><span>{url}</span></a></li>'
                )
            items = f"<ol>{''.join(rows)}</ol>"
        return HISTORY_HTML_TEMPLATE.replace("__HISTORY_ITEMS__", items)

    def load_url(self, raw_url: str) -> None:
        url = raw_url.strip()
        if not url:
            self.load_home()
            return

        if self.is_search_query(url):
            url = f"{GOOGLE_SEARCH_URL}{quote_plus(url)}"

        if not QUrl(url).scheme():
            url = f"https://{url}"

        self.active_web_view().setUrl(QUrl(url))

    def update_address_bar(self, view: BrowserTab, url: QUrl) -> None:
        if view is not self.active_web_view():
            return
        self.address_bar.setText(url.toString())
        self.star_btn.setChecked(False)

    def update_tab_title(self, view: BrowserTab, title: str) -> None:
        index = self.pages.indexOf(view)
        if index != -1:
            widget = self.tab_buttons_layout.itemAt(index).widget()
            if isinstance(widget, TabButton):
                widget.set_title(title or APP_TITLE)
        if view is self.active_web_view():
            self.update_window_title(title)
        self._record_history(view, title)

    def _on_current_tab_changed(self, index: int) -> None:
        self.pages.setCurrentIndex(index)
        view = self.active_web_view()
        url = view.url().toString()
        self.address_bar.setText("" if url == HOME_URL else url)
        self.star_btn.setChecked(False)
        self.update_window_title(view.title())

    def update_window_title(self, title: str) -> None:
        self.setWindowTitle(title or APP_TITLE)

    def _record_history(self, view: BrowserTab, title: str) -> None:
        if not self._recording_history:
            return

        url = view.url().toString()
        if not url or url in {HOME_URL, HISTORY_URL}:
            return
        if self._history and self._history[-1].url == url:
            self._history[-1].title = title or url
            return
        self._history.append(HistoryEntry(title=title or url, url=url))

    # ----------------------------------------------------------- window anim
    def minimize_with_animation(self) -> None:
        """Animate the window shrinking and fading before minimizing.

        Frameless windows on most desktop environments do not get the system
        minimize animation for free, so we fake a Windows-style slide+fade
        toward the bottom of the screen before actually minimizing.
        """
        if self._is_minimizing or self.isMinimized():
            return
        self._is_minimizing = True

        if not self.isMaximized() and not self.isFullScreen():
            self._normal_geometry = self.geometry()
        start_geom = self.geometry()
        target_geom = QRect(
            start_geom.x() + start_geom.width() // 6,
            start_geom.y() + start_geom.height(),
            max(1, start_geom.width() * 2 // 3),
            max(1, start_geom.height() // 2),
        )

        geo_anim = QPropertyAnimation(self, b"geometry", self)
        geo_anim.setDuration(220)
        geo_anim.setStartValue(start_geom)
        geo_anim.setEndValue(target_geom)
        geo_anim.setEasingCurve(QEasingCurve.Type.InCubic)

        op_anim = QPropertyAnimation(self, b"windowOpacity", self)
        op_anim.setDuration(220)
        op_anim.setStartValue(1.0)
        op_anim.setEndValue(0.0)
        op_anim.setEasingCurve(QEasingCurve.Type.InCubic)

        group = QParallelAnimationGroup(self)
        group.addAnimation(geo_anim)
        group.addAnimation(op_anim)
        self._window_anim = group

        def _finalize() -> None:
            # Stay at the small/transparent geometry so when the window manager
            # later restores us, _animate_restore_from_minimized expands cleanly
            # without a flash of the full-size window.
            self._suppress_state_anim = True
            super(BrowserWindow, self).showMinimized()
            self._is_minimizing = False
            self._suppress_state_anim = False

        group.finished.connect(_finalize)
        group.start()

    def _animate_restore_from_minimized(self) -> None:
        target_geom = self._normal_geometry or self.geometry()
        start_geom = QRect(
            target_geom.x() + target_geom.width() // 6,
            target_geom.y() + target_geom.height() // 4,
            max(1, target_geom.width() * 2 // 3),
            max(1, target_geom.height() // 2),
        )
        self.setWindowOpacity(0.0)
        self.setGeometry(start_geom)

        geo_anim = QPropertyAnimation(self, b"geometry", self)
        geo_anim.setDuration(240)
        geo_anim.setStartValue(start_geom)
        geo_anim.setEndValue(target_geom)
        geo_anim.setEasingCurve(QEasingCurve.Type.OutCubic)

        op_anim = QPropertyAnimation(self, b"windowOpacity", self)
        op_anim.setDuration(240)
        op_anim.setStartValue(0.0)
        op_anim.setEndValue(1.0)
        op_anim.setEasingCurve(QEasingCurve.Type.OutCubic)

        group = QParallelAnimationGroup(self)
        group.addAnimation(geo_anim)
        group.addAnimation(op_anim)
        self._window_anim = group
        group.start()

    def changeEvent(self, event) -> None:  # type: ignore[override]
        if event.type() == QEvent.Type.WindowStateChange:
            old_state = event.oldState()
            new_state = self.windowState()
            was_minimized = bool(old_state & Qt.WindowState.WindowMinimized)
            is_minimized = bool(new_state & Qt.WindowState.WindowMinimized)
            if (
                was_minimized
                and not is_minimized
                and not self._suppress_state_anim
            ):
                self._animate_restore_from_minimized()
            if not is_minimized and not self.isMaximized() and not self.isFullScreen():
                self._normal_geometry = self.geometry()
        super().changeEvent(event)

    def showEvent(self, event) -> None:  # type: ignore[override]
        super().showEvent(event)
        if self._normal_geometry is None and not self.isMaximized():
            self._normal_geometry = self.geometry()

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
