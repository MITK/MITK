project(org_mitk_gui_qt_photoacoustics_imageprocessing)

mitk_create_plugin(
  EXPORT_DIRECTIVE IMAGEPROCESSING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkPhotoacousticsAlgorithms
)

IF(MITK_USE_OpenCL)
  add_definitions(-DPHOTOACOUSTICS_USE_GPU)
ENDIF(MITK_USE_OpenCL)