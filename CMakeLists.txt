cmake_minimum_required(VERSION 3.16)

project(minimal_chromium_browser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets WebEngineWidgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets WebEngineWidgets Svg)

add_executable(minimal-browser
    src/main.cpp
)

target_link_libraries(minimal-browser
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::WebEngineWidgets
        Qt${QT_VERSION_MAJOR}::Svg
)

add_custom_command(TARGET minimal-browser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/morphine_logo.png
        $<TARGET_FILE_DIR:minimal-browser>/morphine_logo.png
)

install(TARGETS minimal-browser RUNTIME DESTINATION bin)
install(FILES morphine_logo.png DESTINATION bin)
