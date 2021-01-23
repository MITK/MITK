option(BUILD_CoreCmdApps "Build command-line apps of the MitkCore module" OFF)

if(BUILD_CoreCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(NAME FileConverter)
  mitkFunctionCreateCommandLineApp(NAME ImageTypeConverter)
endif()
