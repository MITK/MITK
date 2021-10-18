mitk_create_module(
  INCLUDE_DIRS Algorithms Controllers DataManagement Interactions Rendering SegmentationUtilities/BooleanOperations SegmentationUtilities/MorphologicalOperations
  DEPENDS MitkAlgorithmsExt MitkSurfaceInterpolation MitkGraphAlgorithms MitkContourModel MitkMultilabel
  PACKAGE_DEPENDS
    PUBLIC ITK|QuadEdgeMesh
    PRIVATE ITK|LabelMap+Watersheds VTK|ImagingGeneral
)

add_subdirectory(Testing)
