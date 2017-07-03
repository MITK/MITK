set(CPP_FILES
  mitkPhotoacousticImage.cpp
  
  Algorithms/mitkPhotoacousticBeamformingFilter.cpp
  
  Algorithms/OCL/mitkPhotoacousticOCLBeamformer.cpp
)

set(RESOURCE_FILES
  DASQuadratic.cl
  DMASQuadratic.cl
  DASspherical.cl
  DMASspherical.cl
  CropFilter.cl
)