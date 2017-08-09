project(org_mitk_gui_qt_extapplication)

set (package_depends "")

if(MITK_USE_Qt5_WebEngine)
  set(package_depends Qt5|WebEngineWidgets)
endif()

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_EXTAPP
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS ${package_depends}
)

if(MITK_USE_Qt5_WebEngine)
  target_compile_definitions(${PLUGIN_TARGET} PRIVATE -DMITK_USE_Qt5_WebEngine)
endif()
