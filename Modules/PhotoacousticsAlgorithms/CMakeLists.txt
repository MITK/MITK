set(dependencies_list MitkCore MitkAlgorithmsExt)

IF(MITK_USE_OpenCL)
  add_definitions(-DPHOTOACOUSTICS_USE_GPU)
  set(dependencies_list ${dependencies_list} MitkOpenCL)
  message("Using OpenCL in PhotoacousticAlgorithms")
ENDIF(MITK_USE_OpenCL)

MITK_CREATE_MODULE(
  SUBPROJECTS
  DEPENDS ${dependencies_list}
  #AUTOLOAD_WITH MitkCore
  INCLUDE_DIRS PUBLIC Algorithms/ITKUltrasound Algorithms Algorithms/OCL
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  PACKAGE_DEPENDS ITK|ITKFFT+ITKImageCompose+ITKImageIntensity
)