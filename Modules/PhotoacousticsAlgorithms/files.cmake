set(CPP_FILES
  mitkPhotoacousticImage.cpp
  
  Algorithms/mitkPhotoacousticBeamformingFilter.cpp
  
  Algorithms/OCL/mitkPhotoacousticOCLBeamformer.cpp
  
  Algorithms/OCL/mitkPhotoacousticBModeFilter.cpp
)

set(RESOURCE_FILES
  DASQuadratic.cl
  DMASQuadratic.cl
  DASspherical.cl
  DMASspherical.cl
  BModeAbs.cl
  BModeAbsLog.cl
)