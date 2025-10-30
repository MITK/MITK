file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkDICOMSegmentationConstants.cpp
  mitkDICOMSegmentationPropertyHelper.cpp
  mitkLabel.cpp
  mitkLabelHighlightGuard.cpp
  mitkLabelSetImage.cpp
  mitkLabelSetImageConverter.cpp
  mitkLabelSetImageHelper.cpp
  mitkLabelSetImageToSurfaceFilter.cpp
  mitkLabelSetImageToSurfaceThreadedFilter.cpp
  mitkLabelSetImageVtkMapper2D.cpp
  mitkLabelSuggestionHelper.cpp
  mitkMultiLabelEvents.cpp
  mitkMultiLabelIOHelper.cpp
  mitkMultilabelObjectFactory.cpp
  mitkMultiLabelPredicateHelper.cpp
  mitkMultiLabelSegmentationVtkMapper3D.cpp
)
