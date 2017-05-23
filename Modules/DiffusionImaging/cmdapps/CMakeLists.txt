if(BUILD_DiffusionCmdApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of diffusion cmdapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( diffusioncmdapps
    NetworkCreation^^MitkFiberTracking_MitkConnectomics
    NetworkStatistics^^MitkConnectomics

    Fiberfox^^MitkFiberTracking
    MultishellMethods^^MitkFiberTracking
    PeaksAngularError^^MitkFiberTracking
    PeakExtraction^^MitkFiberTracking
    FiberExtraction^^MitkFiberTracking
    FiberProcessing^^MitkFiberTracking
    FiberDirectionExtraction^^MitkFiberTracking
    # LocalDirectionalFiberPlausibility^^MitkFiberTracking # HAS TO USE NEW PEAK IMAGE FORMAT
    StreamlineTracking^^MitkFiberTracking
    GibbsTracking^^MitkFiberTracking
    TractometerMetrics^^MitkFiberTracking
    FileFormatConverter^^MitkFiberTracking
    RfTraining^^MitkFiberTracking
    TractDensity^^MitkFiberTracking
    Sift2WeightCopy^^MitkFiberTracking
    )

    foreach(diffusioncmdapp ${diffusioncmdapps})
      # extract cmd app name and dependencies
      string(REPLACE "^^" "\\;" cmdapp_info ${diffusioncmdapp})
      set(cmdapp_info_list ${cmdapp_info})
      list(GET cmdapp_info_list 0 appname)
      list(GET cmdapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkDiffusionCore ${dependencies_list}
        PACKAGE_DEPENDS ITK
      )
    endforeach()

  # This cmd app does not depend on mitkDiffusionImaging at all
  mitkFunctionCreateCommandLineApp(
    NAME Dicom2Nrrd
    DEPENDS MitkCore ${dependencies_list}
  )

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES ${EXECUTABLE_TARGET})
  endif()

  endif()
