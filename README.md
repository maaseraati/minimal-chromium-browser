# Minimal Chromium Browser Prototype

Минимальный desktop-браузерный прототип на C++ и Qt WebEngine.

Проект реализован как нативное Qt-приложение поверх Chromium-based
QtWebEngine: собственный HTML/JS/rendering engine не создаётся.

## Возможности

- отдельное desktop-приложение;
- главное окно с web view;
- стартовая страница `morphine` со строкой поиска Google;
- вкладки с кнопками закрытия и открытием новых вкладок;
- адресная строка;
- открытие URL по `Enter`;
- поиск в Google из адресной строки для обычных текстовых запросов;
- автоматическое добавление `https://`, если протокол не указан;
- кнопки `Back`, `Forward`, `Reload`, `History`;
- переход по ссылкам внутри страницы в том же окне;
- открытие новых окон WebEngine как новых вкладок;
- обновление адресной строки при навигации;
- обновление заголовка окна и вкладки по заголовку текущей страницы;
- история текущей сессии на странице `morphine://history`;
- восстановление последней закрытой вкладки через `Ctrl+Shift+T`;
- горячие клавиши для вкладок, адресной строки, перезагрузки, zoom и fullscreen;
- frameless window с кастомными кнопками свернуть/развернуть/закрыть;
- изменение размера окна по краям.

## Стек

- C++17
- CMake 3.16+
- Qt 5 или Qt 6
- Qt WebEngine / Chromium-based engine
- Qt Svg

## Установка зависимостей

### Ubuntu / Debian

Qt 6:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build qt6-base-dev qt6-webengine-dev qt6-svg-dev
```

Если в дистрибутиве нет Qt 6 WebEngine, можно собрать с Qt 5:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build qtbase5-dev qtwebengine5-dev libqt5svg5-dev
```

## Сборка

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Запуск

```bash
./build/minimal-browser
```

## Установка

```bash
cmake --install build
```

## Структура проекта

```text
.
├── CMakeLists.txt      # CMake-конфигурация C++/Qt приложения
├── src/main.cpp        # основной код браузера
└── README.md           # инструкция
```

## Что не входит в первую C++ версию

- постоянное хранилище истории;
- менеджер загрузок;
- расширения;
- режим инкогнито;
- синхронизация;
- собственный web engine.
