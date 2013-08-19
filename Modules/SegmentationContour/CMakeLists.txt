MITK_CREATE_MODULE( SegmentationContour
  INCLUDE_DIRS Algorithms DataManagement IO Rendering
  DEPENDS Mitk
)

add_subdirectory(Testing)