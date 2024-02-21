cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME common)

file(GLOB_RECURSE COMMON_H_FILES ./*.h)
file(GLOB_RECURSE COMMON_CPP_FILES ./*.cpp)

set(ALL_FILES ${COMMON_H_FILES} ${COMMON_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(
    ${TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(
    ${TARGET_NAME} PUBLIC
    Qt5::Core
    Qt5::Widgets
    KF5::WindowSystem
    KF5::Service
    ${KLOG_QT5_LIBRARIES})
