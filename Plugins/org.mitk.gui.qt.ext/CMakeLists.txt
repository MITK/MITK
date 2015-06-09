project(org_mitk_gui_qt_ext)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_COMMON_EXT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS
    PRIVATE MitkQtWidgetsExt MitkSceneSerialization MitkAppUtil
)
