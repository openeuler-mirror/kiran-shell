cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE H_FILES *.h)
file(GLOB_RECURSE CPP_FILES *.cpp)

set(TARGET_NAME kiran-shell)

set(TRANSLATION_FILES
    "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

# cmake时更新ts文件
execute_process(COMMAND lupdate-qt5 ${H_FILES} ${CPP_FILES} -ts
                        ${TRANSLATION_FILES})

# make时生成qm文件
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

add_executable(${TARGET_NAME} ${H_FILES} ${CPP_FILES} ${QM_FILES}
                              "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/profile
          ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          Qt5::Widgets
          Qt5::DBus
          KF5::WindowSystem
          kiran-integration-theme
          common)

install(FILES ${QM_FILES} DESTINATION ${KS_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
