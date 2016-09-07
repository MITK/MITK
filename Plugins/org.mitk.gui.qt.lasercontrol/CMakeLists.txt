project(org_mitk_gui_qt_lasercontrol)

mitk_create_plugin(
  EXPORT_DIRECTIVE LASERCONTROL_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkPhotoacoustics MitkPhotoacousticsHardwareOpotek
)
