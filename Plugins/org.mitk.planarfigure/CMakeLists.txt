project(org_mitk_planarfigure)

mitk_create_plugin(
  EXPORT_DIRECTIVE ORG_MITK_PLANARFIGURE_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkPlanarFigure MitkQtWidgets
  )
