project(org_blueberry_ui_qt_log)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_log_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)

if(MITK_USE_Qt5)
  qt5_use_modules(${PLUGIN_TARGET} Widgets)
endif()
