set(crashhandling_dep )
if(MITK_USE_sentry)
  set(crashhandling_dep MitkCrashHandling)
endif()

mitk_create_module(
  AUTOMOC
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework Qt6|Widgets Poco|Util
    PRIVATE VTK|GUISupportQt
  DEPENDS
    PUBLIC qtsingleapplication
    PRIVATE MitkCore ${crashhandling_dep}
)

if(MITK_USE_sentry AND TARGET ${MODULE_TARGET})
  target_compile_definitions(${MODULE_TARGET} PRIVATE MITK_HAS_CRASHHANDLING)
endif()
