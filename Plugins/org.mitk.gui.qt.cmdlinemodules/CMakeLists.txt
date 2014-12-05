project(org_mitk_gui_qt_cmdlinemodules)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE CLI_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS CTK|CTKCommandLineModulesFrontendQtGui+CTKCommandLineModulesBackendLocalProcess Qt4|QtUiTools Qt5|UiTools+XmlPatterns
)
