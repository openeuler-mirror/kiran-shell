cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SRC_H_FILES ./*.h)
file(GLOB_RECURSE SRC_CPP_FILES ./*.cpp)

set(TARGET_NAME kiran-shell)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(${TARGET_NAME} ${SRC_H_FILES} ${SRC_CPP_FILES} ${QM_FILES}
                              "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS} ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KIRAN_STYLE_LIBRARIES} ${KLOG_QT5_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES} Qt5::Widgets Qt5::DBus KF5::WindowSystem)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
