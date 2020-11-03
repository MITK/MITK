project(org_mitk_gui_qt_flowapplication)

# see bug-19679
set(additional_dependencies "")

if(APPLE)
 set(additional_dependencies Qt5|DBus)
endif()


mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_FLOW_BENCH_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS
  PRIVATE MitkQtWidgetsExt MitkSceneSerialization MitkAppUtil 
  PACKAGE_DEPENDS Qt5|WebEngineWidgets ${additional_dependencies}
)
