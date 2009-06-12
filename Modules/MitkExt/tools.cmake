# Segmentation (or related) tools for interactive segmentation view
#
# Tool classes, which do not appear in this list but in the CPP_FILES
# list, do not need a factory. The list below contains only "end-user"
# tools, which should be instantiated during runtime
SET( TOOL_FILES
  Interactions/mitkAddContourTool.cpp
  Interactions/mitkAutoCropTool.cpp
  Interactions/mitkBinaryThresholdTool.cpp
  Interactions/mitkCalculateGrayValueStatisticsTool.cpp
  Interactions/mitkCalculateVolumetryTool.cpp
  Interactions/mitkCorrectorTool2D.cpp
  Interactions/mitkCreateSurfaceTool.cpp
  Interactions/mitkEraseRegionTool.cpp
  Interactions/mitkFillRegionTool.cpp
  Interactions/mitkRegionGrowingTool.cpp
  Interactions/mitkSubtractContourTool.cpp
  Interactions/mitkDrawPaintbrushTool.cpp
  Interactions/mitkErasePaintbrushTool.cpp
)
