project(org_mitk_gui_qt_basicimageprocessing)

mitk_create_plugin(
  EXPORT_DIRECTIVE BASICIMAGEPROCESSING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkMapperExt MitkImageDenoising
)
