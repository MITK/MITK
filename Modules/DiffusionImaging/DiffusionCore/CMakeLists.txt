find_package(ITK)
if(ITK_GDCM_DIR)
  include(${ITK_GDCM_DIR}/GDCMConfig.cmake)
  if(GDCM_MAJOR_VERSION EQUAL 2)
    add_definitions(-DGDCM2)
    set(ITK_USES_GDCM2 1)
  endif(GDCM_MAJOR_VERSION EQUAL 2)
endif(ITK_GDCM_DIR)

MITK_CREATE_MODULE( DiffusionCore
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/Reconstruction DicomImport IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkExt SceneSerializationBase QmitkExt
  PACKAGE_DEPENDS Boost
)

add_subdirectory(Testing)