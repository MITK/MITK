# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_dtiatlasapp)

mitk_create_plugin(
  EXPORT_DIRECTIVE DTIATLAS_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkSceneSerialization
  PACKAGE_DEPENDS Qt4|QtWebKit Qt5|WebKitWidgets
)

