option(BUILD_PerfusionCmdApps "Build command-line apps of the Perfusion module" OFF)

if(BUILD_PerfusionCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(NAME CurveDescriptor DEPENDS MitkPharmacokinetics)
  mitkFunctionCreateCommandLineApp(NAME MRPerfusion DEPENDS MitkPharmacokinetics)
  mitkFunctionCreateCommandLineApp(NAME MRSignal2Concentration DEPENDS MitkPharmacokinetics)
endif()
