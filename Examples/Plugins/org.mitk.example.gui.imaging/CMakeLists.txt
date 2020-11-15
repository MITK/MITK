project(org_mitk_example_gui_imaging)

mitk_create_plugin(
  EXPORT_DIRECTIVE EXAMPLES_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkSegmentation
  PACKAGE_DEPENDS VTK|IOImage
)
