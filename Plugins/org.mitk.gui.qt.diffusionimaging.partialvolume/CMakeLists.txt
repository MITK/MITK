# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_diffusionimaging_partialvolume)

mitk_create_plugin(
  SUBPROJECTS MITK-DTI
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_PARTIALVOLUME_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkDiffusionCore MitkFiberTracking
)
