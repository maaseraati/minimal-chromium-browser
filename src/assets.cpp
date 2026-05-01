#include "assets.h"

#include <QCoreApplication>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

namespace morphine {

QString iconSvg(const QString &name, const QString &color)
{
    if (name == "arrow_back") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "arrow_forward") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="m12 4-1.41 1.41L16.17 11H4v2h12.17l-5.58 5.59L12 20l8-8z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "refresh") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M17.65 6.35A7.95 7.95 0 0 0 12 4a8 8 0 1 0 7.45 5h-2.1A6 6 0 1 1 12 6c1.66 0 3.14.69 4.22 1.78L13 11h8V3z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "history") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M13 3a9 9 0 1 1-8.95 8H2l3.1-3.1L8.2 11H6.07A7 7 0 1 0 13 5a6.96 6.96 0 0 0-4.95 2.05L6.64 5.64A8.96 8.96 0 0 1 13 3zm-1 4h1.5v5l4 2.4-.75 1.23L12 12.8z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "shield") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 2 4 5v6.09c0 5.05 3.41 9.76 8 10.91 4.59-1.15 8-5.86 8-10.91V5l-8-3zm0 2.18 6 2.25v4.66c0 4.05-2.7 7.91-6 8.86-3.3-.95-6-4.81-6-8.86V6.43l6-2.25z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "lock") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M17 9h-1V7a4 4 0 0 0-8 0v2H7a2 2 0 0 0-2 2v8a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-8a2 2 0 0 0-2-2zm-7-2a2 2 0 1 1 4 0v2h-4zm7 12H7v-8h10z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "star_outline") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="m22 9.24-7.19-.62L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21 12 17.27 18.18 21l-1.63-7.03zM12 15.4l-3.76 2.27 1-4.28-3.32-2.88 4.38-.38L12 6.1l1.71 4.04 4.38.38-3.32 2.88 1 4.28z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "star_filled") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 17.27 18.18 21l-1.64-7.03L22 9.24l-7.19-.61L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "more_vert") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 8a2 2 0 1 0 0-4 2 2 0 0 0 0 4zm0 2a2 2 0 1 0 0 4 2 2 0 0 0 0-4zm0 6a2 2 0 1 0 0 4 2 2 0 0 0 0-4z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "search") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M9.5 3a6.5 6.5 0 0 1 5.18 10.43l.27.27h.8l5 5-1.5 1.5-5-5v-.8l-.27-.27A6.5 6.5 0 1 1 9.5 3zm0 2a4.5 4.5 0 1 0 0 9 4.5 4.5 0 0 0 0-9z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "github") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M12 2a10 10 0 0 0-3.16 19.49c.5.09.68-.22.68-.48v-1.7c-2.78.6-3.37-1.18-3.37-1.18-.45-1.15-1.11-1.46-1.11-1.46-.91-.62.07-.61.07-.61 1 .07 1.53 1.03 1.53 1.03.9 1.52 2.34 1.08 2.91.82.09-.65.35-1.08.63-1.33-2.22-.25-4.56-1.11-4.56-4.95 0-1.1.39-1.99 1.03-2.69-.1-.25-.45-1.27.1-2.65 0 0 .84-.27 2.75 1.02A9.5 9.5 0 0 1 12 6.98c.85 0 1.7.11 2.5.33 1.9-1.29 2.74-1.02 2.74-1.02.55 1.38.2 2.4.1 2.65.64.7 1.03 1.6 1.03 2.69 0 3.85-2.34 4.69-4.57 4.94.36.31.68.92.68 1.86V21c0 .27.18.58.69.48A10 10 0 0 0 12 2z"/></svg>)SVG")
            .arg(color);
    }
    if (name == "close") {
        return QString::fromUtf8(
                   R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M18.3 5.71 12 12l6.3 6.29-1.41 1.41-6.3-6.29-6.3 6.29-1.41-1.41L9.17 12 2.88 5.71 4.29 4.3l6.3 6.29 6.29-6.29z"/></svg>)SVG")
            .arg(color);
    }
    return QString::fromUtf8(
               R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%1"><path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6z"/></svg>)SVG")
        .arg(color);
}

QIcon svgIcon(const QString &name, const QString &color, int size)
{
    QSvgRenderer renderer(iconSvg(name, color).toUtf8());
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    return QIcon(pixmap);
}

QString htmlEscaped(const QString &value)
{
    QString result = value.toHtmlEscaped();
    result.replace('"', "&quot;");
    return result;
}

QString logoDataUri()
{
    QFile file(QCoreApplication::applicationDirPath() + "/" + LogoFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName(QCoreApplication::applicationDirPath() + "/../" + LogoFileName);
    }
    if (!file.isOpen()) {
        file.setFileName(QCoreApplication::applicationDirPath() + "/../../" + LogoFileName);
    }
    if (!file.isOpen() && !file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return "data:image/png;base64," + QString::fromLatin1(file.readAll().toBase64());
}

bool isSearchQuery(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.contains(' ')) {
        return true;
    }
    return !trimmed.contains('.') && !trimmed.contains(':');
}

QString homeHtml()
{
    const QString logo = logoDataUri();
    const QString logoMarkup = logo.isEmpty()
        ? QStringLiteral(R"(<h1 class="brand fallback"><span>Morphine</span></h1>)")
        : QStringLiteral(R"(<img class="brand" src="%1" alt="Morphine" draggable="false">)").arg(logo);
    return QStringLiteral(R"(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>morphine</title>
  <style>
    * { box-sizing: border-box; }
    :root {
      --accent: #2f7eea;
      --accent-soft: #d8e9ff;
      --accent-mid: #7fb3ff;
      --text: #1e3558;
      --muted: #7186a3;
    }
    body {
      margin: 0;
      min-height: 100vh;
      overflow: hidden;
      display: flex;
      align-items: flex-start;
      justify-content: center;
      padding: 8.5vh 24px 24px;
      font-family: Inter, Roboto, "Segoe UI", sans-serif;
      color: var(--text);
      background:
        radial-gradient(900px 360px at 50% 13%, rgba(47, 126, 234, .10), transparent 68%),
        #f9fcff;
    }
    body::before {
      content: "";
      position: fixed;
      left: -8vw;
      right: -8vw;
      bottom: -14vh;
      height: 50vh;
      background: rgba(47, 126, 234, .16);
      border-radius: 46% 54% 0 0 / 28% 30% 0 0;
      transform: rotate(-3deg);
      pointer-events: none;
    }
    body::after {
      content: "";
      position: fixed;
      left: -10vw;
      right: -10vw;
      bottom: 19vh;
      height: 18vh;
      background: #f9fcff;
      border-radius: 0 0 50% 50% / 0 0 100% 100%;
      transform: rotate(-3deg);
      pointer-events: none;
    }
    main {
      position: relative;
      width: min(720px, 100%);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 30px;
      z-index: 1;
    }
    .brand {
      margin: 0;
      width: min(630px, 86vw);
      height: auto;
      display: block;
    }
    h1.brand {
      width: auto;
      font-size: clamp(70px, 10vw, 104px);
      color: var(--accent-mid);
    }
    form {
      width: min(670px, 100%);
      height: 74px;
      display: flex;
      gap: 14px;
      align-items: center;
      padding: 8px 8px 8px 28px;
      border-radius: 999px;
      background: rgba(255,255,255,.92);
      box-shadow: 0 18px 42px rgba(70, 113, 185, .14), inset 0 0 0 1px rgba(143, 178, 232, .28);
      backdrop-filter: blur(16px);
    }
    .search-icon {
      width: 24px;
      height: 24px;
      color: #7c8ba2;
      flex-shrink: 0;
    }
    input {
      flex: 1;
      min-width: 0;
      height: 56px;
      border: 0;
      outline: none;
      background: transparent;
      color: var(--text);
      font: inherit;
      font-size: 16px;
      font-weight: 520;
    }
    input::placeholder { color: #7f8ea5; }
    button {
      width: 52px;
      height: 52px;
      border: 0;
      border-radius: 999px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      background: linear-gradient(135deg, #73abff, var(--accent));
      color: white;
      font: inherit;
      font-size: 0;
      font-weight: 800;
      cursor: pointer;
      box-shadow: 0 10px 22px rgba(47, 126, 234, .32);
    }
    button::before {
      content: "➜";
      font-size: 28px;
      line-height: 1;
    }
    .chips {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 10px;
      padding: 12px;
      border-radius: 26px;
      background: rgba(255,255,255,.58);
      box-shadow: inset 0 0 0 1px rgba(143,178,232,.18);
      backdrop-filter: blur(14px);
    }
    .chip {
      height: 42px;
      display: inline-flex;
      align-items: center;
      gap: 9px;
      padding: 0 18px;
      border-radius: 999px;
      background: rgba(255,255,255,.66);
      color: #5c6d86;
      text-decoration: none;
      font-size: 15px;
      font-weight: 650;
      box-shadow: inset 0 0 0 1px rgba(143,178,232,.20), 0 8px 18px rgba(70,113,185,.08);
    }
    .chip:hover { background: #fff; color: var(--text); }
    .chip-icon {
      width: 20px;
      height: 20px;
      display: inline-grid;
      place-items: center;
      border-radius: 50%;
      background: rgba(47,126,234,.12);
      color: var(--accent);
      font-size: 13px;
      font-weight: 850;
    }
    .chip.add {
      width: 48px;
      padding: 0;
      justify-content: center;
      font-size: 25px;
      color: var(--accent);
    }
  </style>
</head>
<body>
  <main>
    %1
    <form action="https://www.google.com/search" method="get" role="search">
      <svg class="search-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M9.5 3a6.5 6.5 0 0 1 5.18 10.43l.27.27h.8l5 5-1.5 1.5-5-5v-.8l-.27-.27A6.5 6.5 0 1 1 9.5 3zm0 2a4.5 4.5 0 1 0 0 9 4.5 4.5 0 0 0 0-9z"/></svg>
      <input name="q" type="search" placeholder="Search the web or type a URL" autofocus autocomplete="off">
      <button type="submit">Search</button>
    </form>
    <nav class="chips" aria-label="Quick links">
      <a class="chip" href="https://www.google.com"><span class="chip-icon">G</span>Google</a>
      <a class="chip" href="https://github.com"><span class="chip-icon">⌘</span>GitHub</a>
      <a class="chip" href="https://news.ycombinator.com"><span class="chip-icon">Y</span>Hacker News</a>
      <a class="chip" href="https://wikipedia.org"><span class="chip-icon">W</span>Wikipedia</a>
      <a class="chip add" href="morphine://home" aria-label="Add shortcut">+</a>
    </nav>
  </main>
</body>
</html>)").arg(logoMarkup);
}

}  // namespace morphine
