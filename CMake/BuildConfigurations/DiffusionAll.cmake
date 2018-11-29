message(STATUS "Configuring MITK Diffusion with all Plugins")

# Enable non-optional external dependencies
set(MITK_USE_Vigra ON CACHE BOOL "MITK Use Vigra Library" FORCE)
set(MITK_USE_HDF5 ON CACHE BOOL "MITK Use HDF5 Library" FORCE)
set(MITK_USE_MatchPoint ON CACHE BOOL "" FORCE)
set(MITK_USE_DCMTK ON CACHE BOOL "" FORCE)
set(MITK_USE_DCMQI ON CACHE BOOL "" FORCE)
set(MITK_USE_OpenMP ON CACHE BOOL "" FORCE)
set(MITK_USE_Python ON CACHE BOOL "" FORCE)
set(MITK_USE_BetData ON CACHE BOOL "" FORCE)

set(MITK_BUILD_APP_Diffusion ON CACHE BOOL "Build MITK Diffusion" FORCE)

set(BUILD_DiffusionFiberProcessingCmdApps ON CACHE BOOL "Build commandline tools for diffusion fiber processing" FORCE)
set(BUILD_DiffusionFiberfoxCmdApps ON CACHE BOOL "Build commandline tools for diffusion data simulation (Fiberfox)" FORCE)
set(BUILD_DiffusionMiscCmdApps ON CACHE BOOL "Build miscellaneous commandline tools for diffusion" FORCE)
set(BUILD_DiffusionQuantificationCmdApps ON CACHE BOOL "Build commandline tools for diffusion quantification (IVIM, ADC, ...)" FORCE)
set(BUILD_DiffusionTractographyCmdApps ON CACHE BOOL "Build commandline tools for diffusion fiber tractography" FORCE)
set(BUILD_DiffusionTractographyEvaluationCmdApps ON CACHE BOOL "Build commandline tools for diffusion fiber tractography evaluation" FORCE)
set(BUILD_DiffusionConnectomicsCmdApps ON CACHE BOOL "Build commandline tools for diffusion connectomics" FORCE)
set(BUILD_DiffusionPythonCmdApps ON CACHE BOOL "Build commandline tools for diffusion with python" FORCE)

# Build neither all plugins nor examples
set(BUILD_TESTING ON CACHE BOOL "Build the MITK tests" FORCE)

# Activate in-application help generation
set(MITK_DOXYGEN_GENERATE_QCH_FILES ON CACHE BOOL "Use doxygen to generate Qt compressed help files for MITK docs" FORCE)
set(BLUEBERRY_USE_QT_HELP ON CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)

# Disable console window
set(MITK_SHOW_CONSOLE_WINDOW ON CACHE BOOL "Use this to enable or disable the console window when starting MITK GUI Applications" FORCE)
