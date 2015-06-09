project(org_mitk_gui_qt_diffusionimaging)

mitk_create_plugin(
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkConnectomics MitkFiberTracking MitkDiffusionCore MitkQuantification
  PACKAGE_DEPENDS PUBLIC ITK|ITKDiffusionTensorImage Vigra HDF5
)
