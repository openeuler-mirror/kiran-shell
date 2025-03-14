cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SRC_FILES *.h *.cpp *.ui)

set(TARGET_NAME settingbar)

set(TRANSLATION_FILES
    "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

# 更新ts文件
execute_process(COMMAND lupdate-qt5 -no-obsolete ${SRC_FILES} -ts ${TRANSLATION_FILES})
# 生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/org.freedesktop.UPower.xml
  PROPERTIES CLASSNAME DBusUPowerService NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/org.freedesktop.UPower.Device.xml
  PROPERTIES CLASSNAME DBusUPowerDevice NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Power.xml
  PROPERTIES INCLUDE ${PROJECT_SOURCE_DIR}/dbus/idle-action.h
             CLASSNAME KSPower
             NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  PROPERTIES CLASSNAME KSAppearance NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  PROPERTIES CLASSNAME KSAudio NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  PROPERTIES CLASSNAME KSAudioDevice NO_NAMESPACE TRUE)

qt_add_dbus_interface(
  DBUS_INTERFACES ${PROJECT_SOURCE_DIR}/dbus/org.freedesktop.UPower.xml
  upower_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES ${PROJECT_SOURCE_DIR}/dbus/org.freedesktop.UPower.Device.xml
  upower_device_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Power.xml
  ks_power_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  ks_appearance_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  ks_audio_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  ks_audio_device_interface)

add_library(${TARGET_NAME} SHARED ${SRC_FILES} ${QM_FILES} ${DBUS_INTERFACES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS}
          ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS}
          ${KIRAN_DESKTOP_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE Qt5::Widgets
          KF5::NetworkManagerQt
          ${KLOG_QT5_LIBRARIES}
          ${KIRAN_WIDGETS_QT5_LIBRARIES}
          ${KIRAN_DESKTOP_LIBRARIES}
          kiran-integration-theme
          widgets
          common)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
