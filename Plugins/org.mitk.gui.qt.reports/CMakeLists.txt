project(org_mitk_gui_qt_reports)

set(MODULE_DEPENDS
  MitkQtHtml
  MitkQtWidgets
)

if(TARGET MitkRESTAPI)
  list(APPEND MODULE_DEPENDS MitkRESTAPI)
  set(MITK_HAS_RESTAPI 1)
else()
  set(MITK_HAS_RESTAPI 0)
endif()

configure_file(
  mitkReportsPluginConfig.h.in
  mitkReportsPluginConfig.h
)

mitk_create_plugin(
  EXPORT_DIRECTIVE REPORTS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS ${MODULE_DEPENDS}
  PACKAGE_DEPENDS nlohmann_json
)
