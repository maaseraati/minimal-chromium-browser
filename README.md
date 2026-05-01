# Morphine Browser

Минимальная демо-версия desktop-браузера на C++ и Chromium. Проект не пишет
собственный web engine: рендеринг, JS, networking и sandbox даёт Chromium через
Qt WebEngine, а приложение реализует собственную оболочку, вкладки и навигацию.

## Что уже есть

- C++20 desktop-приложение на Qt 6.2+ Widgets.
- Chromium-based web view через Qt WebEngine.
- Домашняя страница `morphine://home`.
- Адресная строка: URL, домены без `https://`, Google search для текстовых запросов.
- Вкладки: новая вкладка, закрытие, переключение, drag-reorder, favicons.
- Ссылки с `target=_blank` и запросы нового окна открываются в новой вкладке.
- Loading state: progress bar, stop/reload button, индикатор загрузки в заголовке вкладки/окна.
- Bookmarks: `Ctrl+D` добавляет текущую страницу, `Ctrl+B` открывает боковую панель.
- History: последние 100 страниц сохраняются, `Ctrl+H` открывает боковую панель.
- Downloads: файлы автоматически сохраняются в системную папку Downloads, статус виден снизу.
- Find in page: `Ctrl+F` открывает find bar, есть поиск вперёд/назад.
- Session restore: открытые вкладки восстанавливаются между запусками.
- Кнопки `Back`, `Forward`, `Reload`, `Home`, `Go`.
- Горячие клавиши: `Ctrl+T`, `Ctrl+W`, `Ctrl+L`, `Ctrl+Tab`,
  `Ctrl+Shift+Tab`, `Ctrl+D`, `Ctrl+B`, `Ctrl+H`, `Ctrl+F`, `Ctrl+,`,
  `Alt+Left`, `Alt+Right`, `Alt+Home`, `Ctrl+R`, `F5`.
- Общий профиль Chromium с persistent cookies, storage и disk cache.
- Settings: меню (☰) или `Ctrl+,` — настройка home page URL, search URL
  template, downloads directory, restore session, плюс кнопки очистки
  history, bookmarks и cookies/cache.

## Почему такой стек

Для MVP лучше всего подходит **C++20 + Qt 6 + Qt WebEngine**:

- быстрее собрать рабочий браузер, чем на CEF или чистом Chromium;
- Qt уже даёт cross-platform UI, вкладки, menus, packaging и event loop;
- Qt WebEngine использует Chromium, поэтому сайты открываются как в настоящем
  Chromium-based браузере;
- позже можно перейти на CEF, если понадобится больше контроля над multi-process
  моделью, request interception, extensions API или кастомным compositor.

Альтернативы:

| Стек | Когда брать |
| --- | --- |
| Qt 6 + Qt WebEngine | Лучший путь для быстрого C++ desktop MVP |
| CEF + custom UI | Больше контроля, сложнее сборка и интеграция |
| Electron | Быстро, но не C++ и тяжелее по памяти |
| Chromium fork | Максимальный контроль, но очень дорого в поддержке |

## Установка зависимостей

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build pkg-config \
  qt6-base-dev qt6-webengine-dev qt6-webengine-dev-tools \
  libqt6webenginecore6-bin libgl1-mesa-dev libglu1-mesa-dev libxkbcommon-dev
```

### macOS

```bash
brew install cmake ninja qt
```

Если CMake не находит Qt:

```bash
export CMAKE_PREFIX_PATH="$(brew --prefix qt)"
```

### Windows

Установи:

- Visual Studio 2022 с C++ workload;
- CMake;
- Qt 6 с модулем Qt WebEngine.

## Сборка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Запуск

```bash
./build/morphine
```

На Windows бинарь будет в `build/morphine.exe` или в конфигурационной папке
генератора Visual Studio.

## Структура

```text
.
├── CMakeLists.txt
├── README.md
└── src
    ├── browsertab.cpp/.h      # web view + toolbar + домашняя страница
    ├── browserwindow.cpp/.h   # главное окно + управление вкладками
    └── main.cpp               # entrypoint
```

## Как сделать шустрее дальше

Быстрые практичные шаги:

1. **Lazy tabs** — не создавать `QWebEngineView` для фоновых вкладок до первого
   открытия.
2. **Tab freezing/discarding** — выгружать тяжелые неактивные вкладки и
   восстанавливать URL при возврате.
3. **Preload home/new tab** — держать заранее подготовленную домашнюю вкладку.
4. **Disk cache + persistent profile** — уже включено в MVP; дальше можно дать
   пользователю настройки размера cache.
5. **Минимальный chrome UI** — меньше QSS, repaint и сложных layout на каждое
   событие загрузки.
6. **Request blocking** — блокировать трекеры/тяжелую рекламу через interceptor,
   когда появится allow/block list.
7. **Release + LTO** — собирать `Release`, затем включить IPO/LTO в CMake.
8. **Профилирование** — смотреть startup, memory и tab creation через
   `perf`, Instruments, Windows Performance Recorder и Chromium tracing.

Что не стоит делать рано:

- форкать Chromium ради MVP;
- писать собственный renderer/JS engine;
- сразу тащить extensions API;
- делать сложный кастомный UI toolkit вместо Qt.

## Roadmap

- settings page;
- private windows;
- simple ad/tracker blocker;
- downloads shelf with progress/cancel controls.
