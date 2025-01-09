cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)
file(GLOB_RECURSE UI_FILES *.ui)

set(TARGET_NAME calendar)

set(TRANSLATION_FILES
    "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

# 更新ts文件
execute_process(COMMAND lupdate-qt5 ${H_FILES} ${CPP_FILES} ${UI_FILES} -ts
                        ${TRANSLATION_FILES})

# 生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
  PROPERTIES CLASSNAME KSTimeDate NO_NAMESPACE TRUE)

qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
  ks_timedate_interface)

add_library(${TARGET_NAME} SHARED ${H_FILES} ${CPP_FILES} ${UI_FILES}
                                  ${QM_FILES} ${DBUS_INTERFACES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS} ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${KIRAN_WIDGETS_QT5_LIBRARIES}
          Qt5::Core
          Qt5::Widgets
          Qt5::Gui
          Qt5::DBus
          KF5::WindowSystem
          kiran-integration-theme
          widgets
          common)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
