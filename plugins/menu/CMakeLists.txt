cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)
file(GLOB_RECURSE UI_FILES *.ui)

set(TARGET_NAME menu)

set(TRANSLATION_FILES
    "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

# qt_create_translation会导致cmake clean把ts文件删掉 qt_create_translation(QM_FILES
# ${CMAKE_SOURCE_DIR} ${TRANSLATION_FILES})

# 更新ts文件
execute_process(COMMAND lupdate-qt5 ${H_FILES} ${CPP_FILES} ${UI_FILES} -ts
                        ${TRANSLATION_FILES})

# 生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
  PROPERTIES CLASSNAME KSAccounts NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
  PROPERTIES CLASSNAME KSAccountsUser NO_NAMESPACE TRUE)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/org.freedesktop.login1.Manager.xml
  PROPERTIES CLASSNAME Freelogin1Manager
             NO_NAMESPACE TRUE
             INCLUDE ${PROJECT_SOURCE_DIR}/dbus/loginddbustypes.h)
set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/dbus/org.gnome.SessionManager.xml
  PROPERTIES CLASSNAME GnomeSessionManager NO_NAMESPACE TRUE)

qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
  ks_accounts_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES
  ${PROJECT_SOURCE_DIR}/dbus/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
  ks_accounts_user_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES ${PROJECT_SOURCE_DIR}/dbus/org.freedesktop.login1.Manager.xml
  free_login1_manager_interface)
qt_add_dbus_interface(
  DBUS_INTERFACES ${PROJECT_SOURCE_DIR}/dbus/org.gnome.SessionManager.xml
  gnome_session_manager_interface)

add_library(${TARGET_NAME} SHARED ${H_FILES} ${CPP_FILES} ${UI_FILES}
                                  ${QM_FILES} ${DBUS_INTERFACES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS}
          ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${KIRAN_WIDGETS_QT5_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          Qt5::Widgets
          Qt5::DBus
          KF5::WindowSystem
          KF5::Service
          KF5::KIOCore
          KF5::KIOWidgets
          KF5::Activities
          KF5::ActivitiesStats
          kiran-integration-theme
          common
          widgets)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
