project(org_mitk_gui_qt_crashtest)

set(crashhandling_dep )
if(MITK_USE_sentry)
  set(crashhandling_dep MitkCrashHandling)
endif()

mitk_create_plugin(
  EXPORT_DIRECTIVE CRASHTEST_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets ${crashhandling_dep}
)

if(MITK_USE_sentry)
  target_compile_definitions(${PLUGIN_TARGET} PRIVATE MITK_HAS_CRASHHANDLING)
endif()
