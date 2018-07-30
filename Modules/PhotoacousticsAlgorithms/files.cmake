
file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  source/filters/mitkBeamformingFilter.cpp
  source/filters/mitkBeamformingSettings.cpp
  source/filters/mitkImageSliceSelectionFilter.cpp
  source/filters/mitkCastToFloatImageFilter.cpp
  source/filters/mitkCropImageFilter.cpp
  source/filters/mitkBandpassFilter.cpp
  source/OpenCLFilter/mitkPhotoacousticBModeFilter.cpp
  source/utils/mitkPhotoacousticFilterService.cpp
  source/utils/mitkBeamformingUtils.cpp
  source/mitkPhotoacousticMotionCorrectionFilter.cpp
)

IF(MITK_USE_OpenCL)
list(APPEND CPP_FILES
    source/OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.cpp
    source/OpenCLFilter/mitkPhotoacousticOCLUsedLinesCalculation.cpp
    source/OpenCLFilter/mitkPhotoacousticOCLDelayCalculation.cpp
)
ENDIF(MITK_USE_OpenCL)

set(RESOURCE_FILES
  UsedLinesCalculation.cl
  DelayCalculation.cl
  DMAS.cl
  DAS.cl
  sDMAS.cl
)
