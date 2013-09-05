OPTION(BUILD_DiffusionMiniApps "Build commandline tools for diffusion" OFF)

IF(BUILD_DiffusionMiniApps)

  # include necessary modules here MitkExt QmitkExt
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
    TractometerAngularErrorTool.cpp
    TractogramAngularError.cpp
    FiberDirectionExtraction.cpp
    PeakExtraction.cpp
    PeaksAngularError.cpp
  )

  # create an executable foreach tool (only one at the moment)
  FOREACH(tool ${DIFFUSIONMINIAPPS})
    ADD_EXECUTABLE(
    ${tool}
    ${tool}.cpp
    ${DIFFUSIONCORE_ADDITIONAL_FILES}
    )

    TARGET_LINK_LIBRARIES(
      ${tool}
      ${ALL_LIBRARIES} )
  ENDFOREACH(tool)
  ENDIF()

  MITK_INSTALL_TARGETS(EXECUTABLES mitkDiffusionMiniApps )

ENDIF(BUILD_DiffusionMiniApps)
