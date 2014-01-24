option(BUILD_DiffusionMiniApps "Build commandline tools for diffusion" OFF)

if(BUILD_DiffusionMiniApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  project( mitkDiffusionMiniApps )

  # fill in the standalone executables here
  set(DIFFUSIONMINIAPPS
    mitkDiffusionMiniApps
  )

  # set additional files here
  set(DIFFUSIONCORE_ADDITIONAL_FILES
    MiniAppManager.cpp
    FileFormatConverter.cpp
    TensorReconstruction.cpp
    QballReconstruction.cpp
    DiffusionIndices.cpp
    CopyGeometry.cpp
    GibbsTracking.cpp
    StreamlineTracking.cpp
    FiberProcessing.cpp
    LocalDirectionalFiberPlausibility.cpp
    #TractogramAngularError.cpp
    FiberDirectionExtraction.cpp
    PeakExtraction.cpp
    PeaksAngularError.cpp
    MultishellMethods.cpp
    #FiberFoxProcessing.cpp
    ExportShImage.cpp
    NetworkCreation.cpp
    NetworkStatistics.cpp
  )

  # deprecated
#  FOREACH(tool ${DIFFUSIONMINIAPPS})
#    ADD_EXECUTABLE(
#    ${tool}
#    ${tool}.cpp
#    ${DIFFUSIONCORE_ADDITIONAL_FILES}
#    )

#    TARGET_LINK_LIBRARIES(
#      ${tool}
#      ${ALL_LIBRARIES} )
#  ENDFOREACH(tool)

  mitk_create_executable(mitkDiffusionMiniApps
    DEPENDS DiffusionCore FiberTracking Connectomics DiffusionIO
    PACKAGE_DEPENDS ITK|ITKDiffusionTensorImage
  )

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES mitkDiffusionMiniApps )
  endif()

endif()
