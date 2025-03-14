cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME widgets)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)
file(GLOB_RECURSE UI_FILES ./*.ui)

set(ALL_FILES ${H_FILES} ${CPP_FILES} ${UI_FILES})

# 更新ts文件
execute_process(COMMAND lupdate-qt5 -no-obsolete ${ALL_FILES} -ts ${TRANSLATION_FILES})

# 生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                        ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PUBLIC Qt5::Core
         Qt5::Widgets
         Qt5::DBus
         KF5::WindowSystem
         ${KLOG_QT5_LIBRARIES}
         kiran-integration-theme
         common)
