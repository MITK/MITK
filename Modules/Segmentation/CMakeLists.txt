mitk_create_module(
  INCLUDE_DIRS
    Algorithms
    Controllers
    DataManagement
    Interactions
    Rendering
    SegmentationUtilities/BooleanOperations
    SegmentationUtilities/MorphologicalOperations
  PCH
    mitkTool.h
  DEPENDS
    MitkAlgorithmsExt
    MitkBoundingShape
    MitkContourModel
    MitkGraphAlgorithms
    MitkMultilabel
    MitkSurfaceInterpolation
  TARGET_DEPENDS
    PUBLIC
      httplib::httplib
      ITKQuadEdgeMesh
      ITKRegionGrowing
    PRIVATE
      GrowCut
      ITKLabelMap
      ITKMathematicalMorphology
      VTK::ImagingGeneral
)

if(TARGET ${MODULE_TARGET})
  if(MITK_USE_OpenMP)
    target_link_libraries(${MODULE_TARGET} PRIVATE OpenMP::OpenMP_CXX)
  endif()

  add_subdirectory(autoload/IO)
  add_subdirectory(cmdapps)
  add_subdirectory(Testing)
endif()
