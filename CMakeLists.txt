cmake_minimum_required(VERSION 3.16) # XCB require at least CMake 3.16.0

project(kiran-shell VERSION 2.7)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(KS REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets DBus LinguistTools)
find_package(dbusmenu-qt5 REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5Service REQUIRED)
find_package(KF5KIO REQUIRED)
find_package(KF5Activities REQUIRED)
find_package(KF5ActivitiesStats REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRAN_WIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(KIRAN_DESKTOP REQUIRED kiran-desktop)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)

configure_file(ks-config.h.in ${PROJECT_BINARY_DIR}/ks-config.h)

add_subdirectory(data)
add_subdirectory(resources)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(third_party)
