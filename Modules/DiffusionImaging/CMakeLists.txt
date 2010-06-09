FIND_PACKAGE(ITK)
IF(ITK_GDCM_DIR)
  INCLUDE(${ITK_GDCM_DIR}/GDCMConfig.cmake)
  IF(GDCM_MAJOR_VERSION EQUAL 2)
    ADD_DEFINITIONS(-DGDCM2)
    SET(ITK_USES_GDCM2 1)
  ENDIF(GDCM_MAJOR_VERSION EQUAL 2)
ENDIF(ITK_GDCM_DIR)

IF(ITK_USES_GDCM2)
  MESSAGE("Warning: MitkDiffusionImaging is still not compatible to GDCM2, won't build module")
ELSE(ITK_USES_GDCM2)

  MITK_CREATE_MODULE( MitkDiffusionImaging
    INCLUDE_DIRS Algorithms DicomImport IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures Reconstruction Tractography Rendering  ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS Mitk
  #  PACKAGE_DEPENDS Boost
  )

  ADD_SUBDIRECTORY(Testing)
  
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mitkDiffusionImagingConfigure.h.in ${CMAKE_CURRENT_BINARY_DIR}/mitkDiffusionImagingConfigure.h)
ENDIF(ITK_USES_GDCM2)
