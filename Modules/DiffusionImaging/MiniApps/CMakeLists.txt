OPTION(BUILD_DiffusionMiniApps "Build commandline tools for diffusion" OFF)

IF(BUILD_DiffusionMiniApps OR MITK_BUILD_ALL_APPS)

  # include necessary modules here
  MITK_CHECK_MODULE(_RESULT DiffusionCore FiberTracking )
  IF(_RESULT)
    MESSAGE("Warning: DiffusionMiniApps is missing ${_RESULT}")
  ELSE(_RESULT)
  MITK_USE_MODULE( DiffusionCore FiberTracking )

  # needed include directories
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ALL_INCLUDE_DIRECTORIES})

  PROJECT( mitkDiffusionMiniApps )

  # fill in the standalone executables here
  SET(DIFFUSIONMINIAPPS
    mitkDiffusionMiniApps
  )

  # set additional files here
  SET(DIFFUSIONCORE_ADDITIONAL_FILES
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
    FiberFoxProcessing.cpp
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
  DEPENDS DiffusionCore FiberTracking Connectomics
  )

  ENDIF()

  MITK_INSTALL_TARGETS(EXECUTABLES mitkDiffusionMiniApps )

ENDIF(BUILD_DiffusionMiniApps OR MITK_BUILD_ALL_APPS)
