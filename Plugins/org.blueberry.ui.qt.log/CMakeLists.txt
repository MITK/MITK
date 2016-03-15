project(org_blueberry_ui_qt_log)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_log_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)

target_link_libraries(${PLUGIN_TARGET} PUBLIC Qt5::Widgets)
