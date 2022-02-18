project(org_mitk_example_gui_regiongrowing)

mitk_create_plugin(
  EXPORT_DIRECTIVE REGIONGROWING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS ITK|RegionGrowing
  NO_INSTALL
)
