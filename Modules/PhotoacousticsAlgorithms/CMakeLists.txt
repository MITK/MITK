set(dependencies_list MitkCore MitkAlgorithmsExt MitkOpenCVVideoSupport)

IF(MITK_USE_OpenCL)
  add_definitions(-DPHOTOACOUSTICS_USE_GPU)
  set(dependencies_list ${dependencies_list} MitkOpenCL)
  message("Using OpenCL in PhotoacousticAlgorithms")
ENDIF(MITK_USE_OpenCL)

MITK_CREATE_MODULE(
  SUBPROJECTS
  DEPENDS ${dependencies_list}
  #AUTOLOAD_WITH MitkCore
  INCLUDE_DIRS PUBLIC include
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  PACKAGE_DEPENDS ITK|ITKFFT+ITKImageCompose+ITKImageIntensity OpenCV tinyxml
)

add_subdirectory(test)
add_subdirectory(MitkPABeamformingTool)
add_subdirectory(MitkPAResampleCropTool)
