project(org_mitk_gui_qt_fit_inspector)

mitk_create_plugin(
  EXPORT_DIRECTIVE MODELFIT_INSPECTOR_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkModelFit MitkModelFitUI
)
