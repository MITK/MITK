OPTION(BUILD_PhotoacousticPhantomGenerator "Build MiniApp for generating a PA phantom in silico" OFF)

IF(BUILD_PhotoacousticPhantomGenerator)
  PROJECT( MitkPAPhantomGenerator )
    mitk_create_executable(PAPhantomGenerator
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsLib
      PACKAGE_DEPENDS
      CPP_FILES PAPhantomGenerator.cpp)

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
