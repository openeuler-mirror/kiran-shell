cmake_minimum_required(VERSION 3.2)

# gschema.xml
install(
  FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.kiran.shell.gschema.xml
        ${PROJECT_SOURCE_DIR}/data/com.kylinsec.kiran.shell.panel.gschema.xml
        ${PROJECT_SOURCE_DIR}/data/com.kylinsec.kiran.shell.applet.gschema.xml
  DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/glib-2.0/schemas/)
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
