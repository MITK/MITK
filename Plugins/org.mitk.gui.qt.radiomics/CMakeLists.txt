
project(org_mitk_gui_qt_radiomics)

mitk_create_plugin(
  EXPORT_DIRECTIVE RADIOMICS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkCLUtilities MitkBasicImageProcessing MitkMultilabel
  PACKAGE_DEPENDS ITK|ITKMathematicalMorphology
)
