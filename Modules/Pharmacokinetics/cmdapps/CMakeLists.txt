option(BUILD_PerfusionMiniApps "Build commandline tools for the perfusion module" OFF)

if(BUILD_PerfusionMiniApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(NAME CurveDescriptorMiniApp DEPENDS MitkPharmacokinetics)
  mitkFunctionCreateCommandLineApp(NAME MRPerfusionMiniApp DEPENDS MitkPharmacokinetics)
  mitkFunctionCreateCommandLineApp(NAME MRSignal2ConcentrationMiniApp DEPENDS MitkPharmacokinetics)
endif()
