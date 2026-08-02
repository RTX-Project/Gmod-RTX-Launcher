# Project Key Decisions & Architecture (AI Assistant Master Rules)

## 📌 Архитектурные решения (Architecture & Core Systems)

- **Главное окно и Авто-масштабирование (Auto High-DPI Scaling)**: UI создаётся в `ImGui::Begin("RTX Launcher", ...)` с флагами `NoDecoration | NoMove | NoSavedSettings | NoResize`. Приложение автоматически определяет разрешение экрана (`CalculateUiScale`) и адаптирует размеры окна (`860×500` на Full HD, пропорционально масштабируется на 2K/4K) и глобальный масштаб шрифтов/элементов (`io.FontGlobalScale = g_uiScale`).
- **Интерфейс в стиле macOS (Sidebar Navigation & Window Controls)**:
  - Левая боковая панель шириной 240px содержит навигацию («Главная», «Настройки», «Моды»).
  - Кнопки управления окном (Закрыть — красная, Свернуть — жёлтая) отрисовываются вручную через `ImDrawList` с эффектами наведения (иконки x и -) поверх `ImGui::InvisibleButton`.
  - Модифицирован перехватчик `WM_NCHITTEST` (исключена зона `pt.x <= 70`), чтобы системный drag-and-drop окна не блокировал клики по кнопкам macOS в левом верхнем углу.
- **Модульная архитектура UI (Refactored `RenderImGuiUI`)**: Монолитный блок отрисовки в `main.cpp` разбит на отдельные файлы (`UI_Overview.cpp`, `UI_Settings.cpp`, `UI_RtxMods.cpp` и т.д.), что сильно упростило поддержку кода и решило проблемы с областью видимости.
- **Детектор Видеокарт (GPU Detection & Hardware Compatibility)**: Через DXGI опрашивается модель видеокарты (`g_app.gpuInfo = DetectGPU()`). Для карт NVIDIA RTX выводится статус полной совместимости. Для карт AMD Radeon RX, Intel Arc/Iris и устаревших NVIDIA GTX выводится предупреждающий баннер на главной странице и в настройках о том, что полная производительность и стабильность RTX Remix на этой архитектуре не гарантируется.
- **Интеллектуальный Модуль Авто-Обновлений (`LauncherUpdater`)**:
  - Всеядный парсер номеров сборок `ParseBuildNumber` (извлекает наибольшее число из тега релиза, напр. `v0.0.107-alpha` ➔ `Build 107`).
  - Опрос GitHub API через WinHTTP с явным включением протоколов `TLS 1.2` и `TLS 1.3` (`WINHTTP_OPTION_SECURE_PROTOCOLS`).
  - Бесшовное авто-обновление в 1 клик с фоновой поддержкой батника командной строки (`rtx_updater.bat`) для горячей замены исполняемого файла. Батник имеет защиту от блокировки антивирусами (бесконечный цикл `timeout /t 1`, пока файл не освободится).
- **Парсинг Модов через ModDB (`ModDBScraper`)**: Моды больше **НЕ** скачиваются с GitHub Releases из-за ограничений на размер файлов. Лаунчер аппаратно парсит страницу проекта на ModDB (`moddbProjectUrl`), находит зеркала и напрямую качает архивы.
- **Инсталлятор `rtx-installer` (Автономный установщик)**: 
  - Отдельный C++ ImGui проект для загрузки и распаковки портативной сборки игры. 
  - Имеет общий визуальный стиль с основным лаунчером (тёмный градиент, скругленные полупрозрачные карточки `ImGui::BeginChild`). Вкомпилирована правильная иконка приложения `app_icon.ico` через `rtx-installer.rc`.
- **Интеграция с GitHub Pages (Сайт)**: Исходники сайта лежат в папке `docs/`. Скачивание лаунчера с сайта идет по динамической ссылке `releases/latest/download/rtx-installer.exe`, что избавляет от необходимости править сайт при выходе новых патчей. Слайдер ДО/ПОСЛЕ на сайте использует настоящие скриншоты (`rtx_on.jpg` и `rtx_off.jpg`).

## 🛠 Правила Разработки и Кодинга (MANDATORY Rules)

1. **Инкремент билдов (Build Incrementing)**: ПРИ КАЖДОМ новом билде / компиляции лаунчера обязательно увеличивать константу `CURRENT_BUILD_NUMBER` на 1 в файле `LauncherUpdater.h`.
2. **Кодировка UTF-8 (Cyrillic Strings)**: При редактировании файлов (особенно `main.cpp`), содержащих кириллицу, всегда сохранять файл в строгой UTF-8 кодировке во избежание Mojibake (кракозябр).
3. **CI/CD Ограничения**: 
   - Workflow `build-beta.yml` собирает скрытые бета-билды при каждом коммите.
   - Workflow `release.yml` собирает публичный релиз **только** при пуше тега (например, `v0.0.2.6.4`).
4. **Не трогать ссылки на сайте (HTML)**: Все кнопки "Скачать" в `docs/index.html` настроены на `/latest/download/rtx-installer.exe`. Их изменять не нужно при обновлении версий.

## 🗺 Карта Плейсхолдеров (Где менять хардкод)

- **Версия лаунчера (`CURRENT_VERSION` / `CURRENT_BUILD_NUMBER`)**: `rtx-launcher/LauncherUpdater.h` (Строки ~16-17)
- **Целевой репозиторий GitHub (`REPO_OWNER` / `REPO_NAME`)**: `rtx-launcher/LauncherUpdater.h` (Строки ~19-20)
- **Ссылка на страницу модов ModDB (`moddbProjectUrl`)**: `rtx-launcher/main.cpp` (В районе строк 1400-1500)
- **Ссылки на соцсети и внешние ресурсы (ShellExecute)**: `rtx-launcher/UI_Settings.cpp` и `main.cpp`
- **Картинки сайта (ДО / ПОСЛЕ)**: `docs/images/rtx_on.jpg` и `docs/images/rtx_off.jpg`
- **Тексты сайта**: `docs/index.html`

## 📁 Ключевые файлы
- **[main.cpp](file:///c:/Users/user/source/repos/rtx-launcher/rtx-launcher/main.cpp)** – Точка входа, инициализация UI.
- **[UI_Overview.cpp](file:///c:/Users/user/source/repos/rtx-launcher/rtx-launcher/UI_Overview.cpp)** – Главная страница (запуск, статистика).
- **[LauncherUpdater.h](file:///c:/Users/user/source/repos/rtx-launcher/rtx-launcher/LauncherUpdater.h)** – Модуль проверки/скачивания обновлений лаунчера.
- **[ModDBScraper.cpp](file:///c:/Users/user/source/repos/rtx-launcher/rtx-launcher/ModDBScraper.cpp)** – Скрапер для скачивания модов с ModDB.
- **[README_DEV.md](file:///c:/Users/user/source/repos/rtx-launcher/README_DEV.md)** – Инструкция для пользователя.

## 📊 Текущий статус проекта
- Активный номер сборки в коде: **`Build 251`** (Версия **`0.0.2.6.4`**).
- Проект успешно компилируется MSVC C++17 без ошибок.
