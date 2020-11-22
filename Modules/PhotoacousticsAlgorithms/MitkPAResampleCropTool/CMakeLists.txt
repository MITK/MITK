OPTION(BUILD_PhotoacousticResampleCropTool "Build MiniApp for resampling and cropping of images" OFF)

IF(BUILD_PhotoacousticResampleCropTool)
  PROJECT( MitkPAResampleCropTool )
    mitk_create_executable(PAResampleCropTool
      DEPENDS MitkCommandLine MitkCore MitkPhotoacousticsAlgorithms
      PACKAGE_DEPENDS tinyxml2|tinyxml2
      CPP_FILES PAResampleCropTool.cpp)

  if(TARGET ${EXECUTABLE_TARGET})
    target_link_libraries(${MODULE_TARGET}
      PRIVATE tinyxml2::tinyxml2
    )
  endif()

  install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
 ENDIF()
