set(CPP_FILES
  mitkPhotoacousticImage.cpp
  
  Algorithms/mitkPhotoacousticBeamformingFilter.cpp
  
  Algorithms/OCL/mitkPhotoacousticOCLBeamformer.cpp
  
  Algorithms/OCL/mitkPhotoacousticBModeFilter.cpp
)

set(RESOURCE_FILES
  DASQuadratic.cl
  DMASQuadratic.cl
  DASSpherical.cl
  DMASSpherical.cl
  BModeAbs.cl
  BModeAbsLog.cl
)