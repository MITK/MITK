project(org_mitk_gui_qt_cest)

mitk_create_plugin(
  EXPORT_DIRECTIVE CEST_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkImageStatistics MitkImageStatisticsUI MitkPlanarFigure MitkCEST
)
