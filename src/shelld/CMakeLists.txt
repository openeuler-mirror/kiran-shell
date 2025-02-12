cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)

set(TARGET_NAME kiran-shelld)

set(TRANSLATION_FILES
    "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

# cmake时更新ts文件
execute_process(COMMAND lupdate-qt5 ${H_FILES} ${CPP_FILES} -ts
                        ${TRANSLATION_FILES})

# make时生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

# dbus相关接口生成 org.kde.StatusNotifierWatcher.xml
set(statusnotifierwatcherxml
    ${PROJECT_SOURCE_DIR}/dbus/org.kde.StatusNotifierWatcher.xml)
qt_add_dbus_adaptor(
  DBUS_ADAPTOR_SRC
  ${statusnotifierwatcherxml}
  status-notifier-watcher.h
  Kiran::StatusNotifierWatcher)
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
             INCLUDE ${PROJECT_SOURCE_DIR}/lib/common/tray-data-types.h
             CLASSNAME StatusNotifierItemInterface)
qt_add_dbus_interface(DBUS_INTERFACES_SRC ${statusnotifieritemxml}
                      status_notifier_item_interface)

add_executable(${TARGET_NAME} ${H_FILES} ${CPP_FILES} ${QM_FILES} ${DBUS_ADAPTOR_SRC} ${DBUS_INTERFACES_SRC}
                              "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS} ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          Qt5::Widgets
          Qt5::DBus
          KF5::Service
          KF5::CoreAddons
          KF5::ConfigCore
          # ${KIRAN_STYLE_HELPER_LIBRARIES} kiran-integration-theme
          common)

install(FILES kiran-shelld.desktop DESTINATION "/etc/xdg/autostart/")
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
