OPTION(BUILD_FiberTrackingMiniApps "Build commandline tools for fiber tracking" OFF)

IF(BUILD_FiberTrackingMiniApps)

  # include necessary modules here MitkExt QmitkExt
  MITK_CHECK_MODULE(_RESULT DiffusionCore FiberTracking )
  IF(_RESULT)
    MESSAGE("Warning: FiberTrackingMiniApps is missing ${_RESULT}")
  ELSE(_RESULT)
  MITK_USE_MODULE( DiffusionCore FiberTracking )

  # needed include directories
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ALL_INCLUDE_DIRECTORIES})

  PROJECT( mitkFiberTrackingMiniApps )

  # fill in the standalone executables here
  SET(FIBERTRACKINGMINIAPPS
    mitkFiberTrackingMiniApps
  )

  # set additional files here
  SET(FIBERTRACKING_ADDITIONAL_FILES
    MiniAppManager.cpp
    GibbsTracking.cpp
    StreamlineTracking.cpp
    FiberProcessing.cpp
    PeakExtraction.cpp
    TractometerAngularErrorTool.cpp
    TractogramAngularError.cpp
    FiberDirectionExtraction.cpp
    ImageFormatConverter.cpp
  )

  # create an executable foreach tool (only one at the moment)
  FOREACH(tool ${FIBERTRACKINGMINIAPPS})
    ADD_EXECUTABLE(
    ${tool}
    ${tool}.cpp
    ${FIBERTRACKING_ADDITIONAL_FILES}
    )

    TARGET_LINK_LIBRARIES(
      ${tool}
      ${ALL_LIBRARIES} )
  ENDFOREACH(tool)
  ENDIF()

  MITK_INSTALL_TARGETS(EXECUTABLES mitkFiberTrackingMiniApps )

ENDIF(BUILD_FiberTrackingMiniApps)
