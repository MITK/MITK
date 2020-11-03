OPTION(BUILD_PhotoacousticSpectralUnmixing "Build MiniApp for batch generating of photoacoustic spectral unmixing" OFF)

IF(BUILD_PhotoacousticSpectralUnmixing)
  PROJECT( MitkSpectralUnmxing )
    mitk_create_executable(SpectralUnmxingApp
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsLib
      PACKAGE_DEPENDS
      CPP_FILES SpectralUnmixingApp.cpp)

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
