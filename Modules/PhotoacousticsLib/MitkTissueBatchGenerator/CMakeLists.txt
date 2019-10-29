OPTION(BUILD_PhotoacousticTissueBatchGenerator "Build MiniApp for batch generating of photoacoustic tissue" OFF)

IF(BUILD_PhotoacousticTissueBatchGenerator)
  PROJECT( MitkTissueBatchGenerator )
    mitk_create_executable(TissueBatchGenerator
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsLib
      PACKAGE_DEPENDS
      CPP_FILES TissueBatchGenerator.cpp)

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
