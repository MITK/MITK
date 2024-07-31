option(BUILD_CoreCmdApps "Build command-line apps of the MitkCore module" OFF)

if(BUILD_CoreCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(NAME FileConverter)
  mitkFunctionCreateCommandLineApp(NAME ImageTypeConverter)
  mitkFunctionCreateCommandLineApp(NAME Fuse3Dto4DImage)
  mitkFunctionCreateCommandLineApp(NAME Split4Dto3DImages)
endif()
