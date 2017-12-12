project(org_mitk_gui_qt_photoacoustics_simulation)

mitk_create_plugin(
  EXPORT_DIRECTIVE PHOTOACOUSTICSIMULATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkPhotoacousticsLib MitkQtWidgetsExt
  PACKAGE_DEPENDS PUBLIC ITK
)
