cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SHOWDESKTOP_H_FILES ./*.h)
file(GLOB_RECURSE SHOWDESKTOP_CPP_FILES ./*.cpp)

set(TARGET_NAME showdesktop)

add_library(${TARGET_NAME} SHARED ${SHOWDESKTOP_H_FILES}
                                  ${SHOWDESKTOP_CPP_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KIRAN_STYLE_LIBRARIES} ${KLOG_QT5_LIBRARIES}
                         Qt5::Widgets Qt5::DBus KF5::WindowSystem)

install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
