# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_diffusionimaging_connectomics)

mitk_create_plugin(
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_CONNECTOMICS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkConnectomics
)
