cmake_minimum_required(VERSION 3.0)

project(kiran-shell VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(KS REQUIRED)
find_package(PkgConfig REQUIRED)
# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets DBus
# LinguistTools)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets DBus LinguistTools)
find_package(dbusmenu-qt5 REQUIRED)
find_package(KF5WindowSystem)
find_package(KF5Service)
find_package(KF5KIO)
find_package(KF5Activities)
find_package(KF5ActivitiesStats)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRAN_STYLE_HELPER REQUIRED kiran-integration-theme)
pkg_search_module(KIRAN_WIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)

configure_file(ks-config.h.in ${PROJECT_BINARY_DIR}/ks-config.h)

add_subdirectory(data)
add_subdirectory(resources)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugins)
