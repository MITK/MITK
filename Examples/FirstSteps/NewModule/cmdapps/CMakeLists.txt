option(BUILD_NewModuleCmdApps "Build command-line apps of the example module" OFF)

if(BUILD_NewModuleCmdApps)
  mitkFunctionCreateCommandLineApp(
    NAME ExampleToUpperCase
    DEPENDS MitkNewModule
    NO_INSTALL
  )
endif()
