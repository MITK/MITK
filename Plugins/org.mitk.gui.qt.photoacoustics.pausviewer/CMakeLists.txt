# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_photoacoustics_pausviewer)

mitk_create_plugin(
  EXPORT_DIRECTIVE PHOTOACOUSTICS_PAUSVIEWER_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkAnnotation
)
