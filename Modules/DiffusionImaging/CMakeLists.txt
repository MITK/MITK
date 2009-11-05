IF(GDCM_MAJOR_VERSION EQUAL 2)
  ADD_DEFINITIONS(-DGDCM2)
ENDIF(GDCM_MAJOR_VERSION EQUAL 2)

MITK_CREATE_MODULE( MitkDiffusionImaging
  INCLUDE_DIRS Algorithms DicomImport IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures Reconstruction Tractography Rendering
  DEPENDS Mitk
#  PACKAGE_DEPENDS Boost
)

ADD_SUBDIRECTORY(Testing)