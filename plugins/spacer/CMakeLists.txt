cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)


set(TARGET_NAME spacer)

add_library(
    ${TARGET_NAME} SHARED
    ${H_FILES}
    ${CPP_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${KLOG_QT5_INCLUDE_DIRS}
  ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS}
)

target_link_libraries(
  ${TARGET_NAME} PRIVATE
  ${KLOG_QT5_LIBRARIES}
  ${KIRAN_WIDGETS_QT5_LIBRARIES}
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  kiran-integration-theme
)

install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
