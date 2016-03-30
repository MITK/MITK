project(org_mitk_matchpoint_core_helper)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_MATCHPOINT_CORE_HELPER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkQtWidgetsExt MitkMatchPointRegistration
  PACKAGE_DEPENDS Qt5|UiTools
)
