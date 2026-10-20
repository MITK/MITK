# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_measurementtoolbox)

set(MODULE_DEPENDS
  MitkQtWidgetsExt
  MitkImageStatistics
  MitkImageStatisticsUI
  MitkPlanarFigure
)

if(TARGET MitkRESTAPI)
  list(APPEND MODULE_DEPENDS MitkRESTAPI)
  set(MITK_HAS_RESTAPI 1)
else()
  set(MITK_HAS_RESTAPI 0)
endif()

configure_file(
  mitkMeasurementToolboxPluginConfig.h.in
  mitkMeasurementToolboxPluginConfig.h
)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_MEASUREMENTTOOLBOX
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS ${MODULE_DEPENDS}
  PACKAGE_DEPENDS nlohmann_json
)
