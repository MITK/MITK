project(org_mitk_gui_qt_cmdlinemodules)

mitk_create_plugin(
  EXPORT_DIRECTIVE CLI_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS
    PUBLIC  CTK|CTKWidgets
    PRIVATE CTK|CTKCommandLineModulesFrontendQtGui+CTKCommandLineModulesBackendLocalProcess Qt4|QtUiTools Qt5|UiTools+XmlPatterns
)
