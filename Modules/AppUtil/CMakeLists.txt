set(crashhandling_dep )
if(MITK_USE_sentry)
  set(crashhandling_dep MitkCrashHandling)
endif()

#[[ MitkCore and MitkCrashHandling are public: the crash-dump UI headers
    expose mitk::CrashDumpInfo and declare a micro service interface. ]]
mitk_create_module(
  AUTOMOC
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework Qt6|Widgets Poco|Util
    PRIVATE VTK|GUISupportQt
  DEPENDS
    PUBLIC qtsingleapplication MitkCore ${crashhandling_dep}
)

if(MITK_USE_sentry AND TARGET ${MODULE_TARGET})
  target_compile_definitions(${MODULE_TARGET} PRIVATE MITK_HAS_CRASHHANDLING)

  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
