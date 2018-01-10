file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  source/mitkPhotoacousticImage.cpp
  source/mitkPhotoacousticBeamformingFilter.cpp
  source/OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.cpp
  source/OpenCLFilter/mitkPhotoacousticBModeFilter.cpp
  source/OpenCLFilter/mitkPhotoacousticOCLUsedLinesCalculation.cpp
  source/OpenCLFilter/mitkPhotoacousticOCLDelayCalculation.cpp
)

set(RESOURCE_FILES
  BModeAbs.cl
  BModeAbsLog.cl
  UsedLinesCalculation.cl
  DelayCalculation.cl
  DMAS.cl
  DAS.cl
  sDMAS.cl
)
