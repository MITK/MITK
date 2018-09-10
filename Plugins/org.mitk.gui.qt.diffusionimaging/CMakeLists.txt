project(org_mitk_gui_qt_diffusionimaging)

mitk_create_plugin(
  SUBPROJECTS MITK-DTI
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkDiffusionCore MitkQuantification MitkConnectomics
)
