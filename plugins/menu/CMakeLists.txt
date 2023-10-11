cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE MENU_H_FILES ./*.h)
file(GLOB_RECURSE MENU_CPP_FILES ./*.cpp)

set(TARGET_NAME menu)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

qt5_wrap_ui(
  UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/window.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/apps-overview.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/recent-files-overview.ui)

add_library(${TARGET_NAME} SHARED ${MENU_H_FILES} ${MENU_CPP_FILES} ${QM_FILES}
                                  ${UI_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KIRAN_STYLE_LIBRARIES}
          ${KLOG_QT5_LIBRARIES}
          Qt5::Widgets
          Qt5::DBus
          KF5::WindowSystem
          KF5::Service
          lib-widgets)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
