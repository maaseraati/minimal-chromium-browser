# Minimal Chromium Browser Prototype

Минимальный desktop-браузерный прототип на базе Chromium.

Проект реализован как простая оболочка поверх QtWebEngine: используется готовый
Chromium-based web engine, собственный HTML/JS/rendering engine не создаётся.

## Возможности

- отдельное desktop-приложение;
- главное окно с web view;
- стартовая страница `https://example.com`;
- адресная строка;
- открытие URL по `Enter` или кнопке `Go`;
- автоматическое добавление `https://`, если протокол не указан;
- кнопки `Back`, `Forward`, `Reload`;
- переход по ссылкам внутри страницы в том же окне;
- обновление адресной строки при навигации;
- обновление заголовка окна по заголовку текущей страницы;
- стандартная обработка ошибок загрузки через QtWebEngine без падения приложения.

## Стек

- Python 3.10+
- PyQt6
- PyQt6-WebEngine / QtWebEngine, Chromium-based engine

## Зависимости

Python-зависимости перечислены в:

- `requirements.txt`
- `pyproject.toml`

Минимальный набор:

```text
PyQt6>=6.7,<7
PyQt6-WebEngine>=6.7,<7
```

## Установка

### Linux / macOS

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

### Windows

```powershell
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

Если PowerShell блокирует активацию virtualenv, выполните:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

## Запуск

### Из исходников

```bash
python browser.py
```

На Windows:

```powershell
python browser.py
```

### Как установленный script entrypoint

```bash
python -m pip install -e .
minimal-browser
```

## Сборка исполняемого файла

Для раннего прототипа обязательная сборка `.exe` не требуется, но её можно
сделать через PyInstaller.

```bash
python -m pip install pyinstaller
pyinstaller --name MinimalBrowser --windowed --onefile browser.py
```

Готовый файл появится в папке `dist/`.

На Linux может понадобиться запуск без `--windowed`, если нужно видеть
диагностический вывод:

```bash
pyinstaller --name MinimalBrowser --onefile browser.py
```

## Структура проекта

```text
.
├── browser.py          # основной код приложения
├── pyproject.toml      # метаданные и зависимости проекта
├── requirements.txt    # зависимости для pip
└── README.md           # инструкция
```

## Что не входит в первую версию

- вкладки;
- закладки;
- история посещений как отдельный UI;
- менеджер загрузок;
- расширения;
- режим инкогнито;
- профили пользователей;
- сохранение паролей;
- синхронизация;
- собственный web engine.

## Дальнейшее развитие

Текущая архитектура специально оставлена простой, чтобы позже добавить:

- вкладки;
- историю;
- закладки;
- страницу настроек;
- домашнюю страницу;
- загрузки;
- поиск по странице.
