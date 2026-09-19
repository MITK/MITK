project(org_blueberry_ui_qt_log)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_blueberry_ui_qt_log_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS PUBLIC Qt6|Widgets
)

# Every log message carries the absolute __FILE__ of its origin. The log view
# shows those paths relative to the source tree, which it can only do if it
# knows where the sources are.
target_compile_definitions(${PLUGIN_TARGET} PRIVATE MITK_SOURCE_DIR="${MITK_SOURCE_DIR}")
