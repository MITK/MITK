OPTION(BUILD_PhotoacousticResampleCropTool "Build MiniApp for resampling and cropping of images" OFF)

IF(BUILD_PhotoacousticResampleCropTool)
  PROJECT( MitkPAResampleCropTool )
    mitk_create_executable(PAResampleCropTool
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsAlgorithms
      PACKAGE_DEPENDS tinyxml
      CPP_FILES PAResampleCropTool.cpp)

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
