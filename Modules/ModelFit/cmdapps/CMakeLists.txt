option(BUILD_ModelFitCmdApps "Build command-line apps of the MitkModelFit module" OFF)

if(BUILD_ModelFitCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(
    NAME GenericFitting
    DEPENDS MitkModelFit
  )
  mitkFunctionCreateCommandLineApp(
    NAME PixelDump
    DEPENDS MitkModelFit
  )
endif()
