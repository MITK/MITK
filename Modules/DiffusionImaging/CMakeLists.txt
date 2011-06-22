FIND_PACKAGE(ITK)
IF(ITK_GDCM_DIR)
  INCLUDE(${ITK_GDCM_DIR}/GDCMConfig.cmake)
  IF(GDCM_MAJOR_VERSION EQUAL 2)
    ADD_DEFINITIONS(-DGDCM2)
    SET(ITK_USES_GDCM2 1)
  ENDIF(GDCM_MAJOR_VERSION EQUAL 2)
ENDIF(ITK_GDCM_DIR)

MITK_CREATE_MODULE( MitkDiffusionImaging
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms DicomImport Interactions IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures/FiberBundle IODataStructures/PlanarFigureComposite IODataStructures Reconstruction Tractography Rendering  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkExt SceneSerializationBase
  PACKAGE_DEPENDS Boost
)

ADD_SUBDIRECTORY(Testing)
  
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mitkDiffusionImagingConfigure.h.in ${CMAKE_CURRENT_BINARY_DIR}/mitkDiffusionImagingConfigure.h)

