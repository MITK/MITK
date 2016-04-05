option(BUILD_DiffusionMiniApps "Build commandline tools for diffusion" OFF)

if(BUILD_DiffusionMiniApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of diffusion miniapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( diffusionminiapps
    DwiDenoising^^
    ImageResampler^^
    NetworkCreation^^MitkFiberTracking_MitkConnectomics
    NetworkStatistics^^MitkConnectomics
    ExportShImage^^
    Fiberfox^^MitkFiberTracking
    MultishellMethods^^MitkFiberTracking
    PeaksAngularError^^MitkFiberTracking
    PeakExtraction^^MitkFiberTracking
    FiberExtraction^^MitkFiberTracking
    FiberProcessing^^MitkFiberTracking
    FiberDirectionExtraction^^MitkFiberTracking
    LocalDirectionalFiberPlausibility^^MitkFiberTracking
    StreamlineTracking^^MitkFiberTracking
    GibbsTracking^^MitkFiberTracking
    CopyGeometry^^
    DiffusionIndices^^
    TractometerMetrics^^MitkFiberTracking
    QballReconstruction^^
    Registration^^
    FileFormatConverter^^MitkFiberTracking
    TensorReconstruction^^
    TensorDerivedMapsExtraction^^
    DiffusionDICOMLoader^^
    DFTraining^^MitkFiberTracking
    DFTracking^^MitkFiberTracking
    )

    foreach(diffusionminiapp ${diffusionminiapps})
      # extract mini app name and dependencies
      string(REPLACE "^^" "\\;" miniapp_info ${diffusionminiapp})
      set(miniapp_info_list ${miniapp_info})
      list(GET miniapp_info_list 0 appname)
      list(GET miniapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkDiffusionCore ${dependencies_list}
        PACKAGE_DEPENDS ITK
      )
    endforeach()

  # This mini app does not depend on mitkDiffusionImaging at all
  mitkFunctionCreateCommandLineApp(
    NAME Dicom2Nrrd
    DEPENDS MitkCore ${dependencies_list}
  )

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES ${EXECUTABLE_TARGET})
  endif()

  endif()
