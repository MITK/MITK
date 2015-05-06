project(org_blueberry_ui_qt_log)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_log_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)

if(MITK_USE_Qt5)
  target_link_libraries(${PLUGIN_TARGET} PUBLIC Qt5::Widgets)
endif()
