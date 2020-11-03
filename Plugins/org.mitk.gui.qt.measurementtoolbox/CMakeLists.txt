# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_measurementtoolbox)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_MEASUREMENTTOOLBOX
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkImageStatistics MitkImageStatisticsUI MitkPlanarFigure MitkChart
)