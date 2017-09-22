OPTION(BUILD_PhotoacousticSimulationMCxyz "Build MiniApp for Photoacoustic Simulation Module (mcxyz)" ON)

IF(BUILD_PhotoacousticSimulationMCxyz)
  PROJECT( MitkMCxyz )
    mitk_create_executable(MCxyz
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsLib
      PACKAGE_DEPENDS
      CPP_FILES MCxyz.cpp)

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
