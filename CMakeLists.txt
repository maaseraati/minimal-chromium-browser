cmake_minimum_required(VERSION 3.21)

project(MorphineBrowser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel Svg)

qt_add_executable(morphine
    resources/resources.qrc
    src/main.cpp
    src/browserpage.cpp
    src/browserpage.h
    src/browserwindow.cpp
    src/browserwindow.h
    src/browsertab.cpp
    src/browsertab.h
    src/chrometabbar.cpp
    src/chrometabbar.h
    src/iconutils.cpp
    src/iconutils.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/thememanager.cpp
    src/thememanager.h
)

target_link_libraries(morphine
    PRIVATE
        Qt6::Widgets
        Qt6::WebEngineWidgets
        Qt6::WebChannel
        Qt6::Svg
)

if(APPLE)
    set_target_properties(morphine PROPERTIES MACOSX_BUNDLE TRUE)
elseif(WIN32)
    set_target_properties(morphine PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
