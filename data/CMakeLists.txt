cmake_minimum_required(VERSION 3.0)

# gschema.xml
install(FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.kiran.panel.gschema.xml
          DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/glib-2.0/schemas/)
install(
  CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_FULL_DATADIR}/glib-2.0/schemas/\")"
)

# layout
install(FILES ${PROJECT_SOURCE_DIR}/data/default.layout
        DESTINATION ${KS_INSTALL_DATADIR}/layouts)