file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE TPP_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.tpp")

set(CPP_FILES
  mitkMAPRegistrationWrapper.cpp
  mitkMAPRegistrationWrapperObjectFactory.cpp
  mitkRegEvaluationObjectFactory.cpp
  mitkRegEvaluationObject.cpp
  Helper/mitkUIDHelper.cpp
  Helper/mitkMAPAlgorithmHelper.cpp
  Helper/mitkMaskedAlgorithmHelper.cpp
  Helper/mitkRegistrationHelper.cpp
  Helper/mitkImageMappingHelper.cpp
  Helper/mitkImageStitchingHelper.cpp
  Helper/mitkPointSetMappingHelper.cpp
  Helper/mitkResultNodeGenerationHelper.cpp
  Helper/mitkTimeFramesRegistrationHelper.cpp
  Rendering/mitkRegistrationWrapperMapper2D.cpp
  Rendering/mitkRegistrationWrapperMapper3D.cpp
  Rendering/mitkRegistrationWrapperMapperBase.cpp
  Rendering/mitkRegEvaluationMapper2D.cpp
  Rendering/mitkRegVisStyleProperty.cpp
  Rendering/mitkRegVisDirectionProperty.cpp
  Rendering/mitkRegVisColorStyleProperty.cpp
  Rendering/mitkRegVisPropertyTags.cpp
  Rendering/mitkRegVisHelper.cpp
  Rendering/mitkRegEvalStyleProperty.cpp
  Rendering/mitkRegEvalWipeStyleProperty.cpp
)

set(MOC_H_FILES
)
