cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES ./*.h)
file(GLOB_RECURSE CPP_FILES ./*.cpp)
file(GLOB_RECURSE UI_FILES ./*.ui)

set(TARGET_NAME taskbar)

# 更新ts文件
execute_process(
    COMMAND lupdate-qt5 ${H_FILES} ${CPP_FILES} ${UI_FILES} -ts ${TRANSLATION_FILES}
)

# 生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})


add_library(
    ${TARGET_NAME} SHARED
    ${H_FILES}
    ${CPP_FILES}
    ${UI_FILES})

target_include_directories(
    ${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${KLOG_QT5_INCLUDE_DIRS}
    ${KIRAN_STYLE_HELPER_INCLUDE_DIRS}
    ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS})

target_link_libraries(
    ${TARGET_NAME} PRIVATE
    ${KLOG_QT5_LIBRARIES}
    ${KIRAN_STYLE_LIBRARIES}
    ${KIRAN_WIDGETS_QT5_LIBRARIES}
    Qt5::Widgets
    Qt5::DBus
    KF5::WindowSystem
    KF5::Service
    KF5::Activities
    KF5::ActivitiesStats
    common)

install(TARGETS ${TARGET_NAME} DESTINATION ${KS_INSTALL_PLUGINDIR})
