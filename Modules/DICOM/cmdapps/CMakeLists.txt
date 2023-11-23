option(BUILD_DICOMCmdApps "Build command-line apps of the MitkDICOM module" OFF)

if(BUILD_DICOMCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(
    NAME DICOMVolumeDiagnostics
    DEPENDS MitkDICOM
  )
endif()
