option(BUILD_PETCmdApps "Build command-line apps of the PET module" OFF)

if(BUILD_PETCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(
    NAME PETSUVCalculation
    DEPENDS MitkPET
  )
endif()
