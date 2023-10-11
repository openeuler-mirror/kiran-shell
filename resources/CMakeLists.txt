file(GLOB_RECURSE SVG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor/*.svg)

install(
  FILES ${SVG_FILES}
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps/")
