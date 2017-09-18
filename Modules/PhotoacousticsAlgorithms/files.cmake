file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  source/mitkPhotoacousticImage.cpp
  
  source/mitkPhotoacousticBeamformingFilter.cpp
  
  source/OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.cpp
  source/OpenCLFilter/mitkPhotoacousticBModeFilter.cpp
  source/OpenCLFilter/mitkPhotoacousticOCLUsedLinesCalculation.cpp
  source/OpenCLFilter/mitkPhotoacousticOCLDelayCalculation.cpp
  source/OpenCLFilter/mitkPhotoacousticOCLMemoryLocSum.cpp
)

set(RESOURCE_FILES
  DASQuadratic.cl
  DASSpherical.cl
  BModeAbs.cl
  BModeAbsLog.cl
  UsedLinesCalculation.cl
  MemoryLocSum.cl
  DelayCalculation.cl
  DMAS.cl
  DAS.cl
)