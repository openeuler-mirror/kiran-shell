cmake_minimum_required(VERSION 3.2)

# gschema.xml
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/schemas/
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/glib-2.0/schemas)
install(
  CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_FULL_DATADIR}/glib-2.0/schemas/\")"
)

# layout
install(FILES ${PROJECT_SOURCE_DIR}/data/default.layout
        DESTINATION ${KS_INSTALL_DATADIR}/layouts)

configure_file(${PROJECT_SOURCE_DIR}/data/kiran-shell.desktop.in
               ${PROJECT_BINARY_DIR}/data/kiran-shell.desktop)
install(FILES ${PROJECT_BINARY_DIR}/data/kiran-shell.desktop
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/)

# libpinyin user.conf 这是一个空文件，如果没有这个文件，libpinyin会报错
install(FILES ${PROJECT_SOURCE_DIR}/data/user.conf
        DESTINATION ${KS_INSTALL_DATADIR})

install(
  FILES ${PROJECT_SOURCE_DIR}/data/applications-menu/kiran-applications.menu
  DESTINATION /etc/xdg/menus)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/applications-menu/directories/
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/desktop-directories)
