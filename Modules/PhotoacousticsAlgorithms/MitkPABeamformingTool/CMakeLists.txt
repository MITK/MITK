OPTION(BUILD_PhotoacousticBeamformingTool "Build MiniApp for beamforming of a PA image" OFF)

IF(BUILD_PhotoacousticBeamformingTool)
  PROJECT( MitkPABeamformingTool )
    mitk_create_executable(PABeamformingTool
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsAlgorithms
      PACKAGE_DEPENDS tinyxml2|tinyxml2
      CPP_FILES PABeamformingTool.cpp)

  if(TARGET ${EXECUTABLE_TARGET})
    target_link_libraries(${MODULE_TARGET}
      PRIVATE tinyxml2::tinyxml2
    )
  endif()

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
