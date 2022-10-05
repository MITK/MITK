mitk_create_module(
  INCLUDE_DIRS Algorithms Controllers DataManagement Interactions Rendering SegmentationUtilities/BooleanOperations SegmentationUtilities/MorphologicalOperations
  DEPENDS MitkAlgorithmsExt MitkSurfaceInterpolation MitkGraphAlgorithms MitkContourModel MitkMultilabel
  PACKAGE_DEPENDS
    PUBLIC ITK|QuadEdgeMesh+RegionGrowing
    PRIVATE ITK|LabelMap+MathematicalMorphology VTK|ImagingGeneral
  TARGET_DEPENDS PRIVATE GrowCut
)

add_subdirectory(Testing)
