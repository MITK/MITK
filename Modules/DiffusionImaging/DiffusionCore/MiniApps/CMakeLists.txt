OPTION(BUILD_DiffusionCoreMiniApps "Build commandline tools for diffusion core" OFF)

IF(BUILD_DiffusionCoreMiniApps)

  # include necessary modules here MitkExt QmitkExt
  MITK_CHECK_MODULE(_RESULT DiffusionCore )
  IF(_RESULT)
    MESSAGE("Warning: DiffusionCoreMiniApps is missing ${_RESULT}")
  ELSE(_RESULT)
  MITK_USE_MODULE( DiffusionCore )

  # needed include directories
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ALL_INCLUDE_DIRECTORIES})

  PROJECT( mitkDiffusionCoreMiniApps )

  # fill in the standalone executables here
  SET(DIFFUSIONCOREMINIAPPS
    mitkDiffusionCoreMiniApps
  )

  # set additional files here
  SET(DIFFUSIONCORE_ADDITIONAL_FILES
    MiniAppManager.cpp
    ImageFormatConverter.cpp
    TensorReconstruction.cpp
    QballReconstruction.cpp
    DiffusionIndices.cpp
    CopyGeometry.cpp
  )

  # create an executable foreach tool (only one at the moment)
  FOREACH(tool ${DIFFUSIONCOREMINIAPPS})
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

  MITK_INSTALL_TARGETS(EXECUTABLES mitkDiffusionCoreMiniApps )

ENDIF(BUILD_DiffusionCoreMiniApps)
