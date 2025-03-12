cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME common)

file(GLOB_RECURSE COMMON_H_FILES *.h)
file(GLOB_RECURSE COMMON_CPP_FILES *.cpp)

set(ALL_FILES ${COMMON_H_FILES} ${COMMON_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

# 获取 libpinyin pkgdatadir
execute_process(
  COMMAND pkg-config --variable=pkgdatadir libpinyin
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE LIBPINYIN_PKGDATADIR)
# 添加libpinyin pkgdatadir到编译选项
add_definitions(-DLIBPINYIN_PKGDATADIR="${LIBPINYIN_PKGDATADIR}/data")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS}
          ${LIBPINYIN_INCLUDE_DIRS}
          ${LIBNOTIFY_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${X11_XCB_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE Qt5::Core
          Qt5::X11Extras
          Qt5::DBus
          KF5::WindowSystem
          KF5::Service
          KF5::KIOWidgets
          KF5::Activities
          ${KLOG_QT5_LIBRARIES}
          ${LIBPINYIN_LIBRARIES}
          ${LIBNOTIFY_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          XCB::XCB
          XCB::IMAGE)
