cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)
file(GLOB_RECURSE UI_FILES *.ui)

set(TARGET_NAME systemtray)

set(TRANSLATION_FILES
    "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

# 更新ts文件
execute_process(COMMAND lupdate-qt5 ${H_FILES} ${CPP_FILES} ${UI_FILES} -ts
                        ${TRANSLATION_FILES})

# 生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

# dbus相关接口生成 org.kde.StatusNotifierWatcher.xml
set(statusnotifierwatcherxml
    ${PROJECT_SOURCE_DIR}/dbus/org.kde.StatusNotifierWatcher.xml)
qt_add_dbus_adaptor(
  DBUS_ADAPTOR_SRC
  ${statusnotifierwatcherxml}
  ${CMAKE_SOURCE_DIR}/plugins/systemtray/status-notifier-watcher.h
  Kiran::Systemtray::StatusNotifierWatcher
  status_notifier_watcher_adaptor
  StatusNotifierWatcherAdaptor)
set_source_files_properties(
  ${statusnotifierwatcherxml}
  PROPERTIES NO_NAMESPACE TRUE CLASSNAME StatusNotifierWatcherInterface)
qt_add_dbus_interface(DBUS_INTERFACES_SRC ${statusnotifierwatcherxml}
                      status_notifier_watcher_interface)

set(statusnotifieritemxml
    ${PROJECT_SOURCE_DIR}/dbus/org.kde.StatusNotifierItem.xml)
set_source_files_properties(
  ${statusnotifieritemxml}
  PROPERTIES NO_NAMESPACE TRUE
             INCLUDE "tray-data-types.h"
             CLASSNAME StatusNotifierItemInterface)
qt_add_dbus_interface(DBUS_INTERFACES_SRC ${statusnotifieritemxml}
                      status_notifier_item_interface)

add_library(
  ${TARGET_NAME} SHARED
  ${H_FILES}
  ${CPP_FILES}
  ${UI_FILES}
  ${DBUS_SOURCES}
  ${QM_FILES}
  ${DBUS_ADAPTOR_SRC}
  ${DBUS_INTERFACES_SRC})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS} ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${KIRAN_WIDGETS_QT5_LIBRARIES}
          Qt5::Widgets
          Qt5::DBus
          KF5::WindowSystem
          dbusmenu-qt5
          kiran-integration-theme
          common
          widgets)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
