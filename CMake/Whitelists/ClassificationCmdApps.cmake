include(${CMAKE_CURRENT_LIST_DIR}/CoreCmdApps.cmake)

list(APPEND enabled_modules
  Classification
  CLCore
  CLVigraRandomForest
  CLUtilities
  CLMRUtilities
  CLImportanceWeighting
  Segmentation
  DataCollection
  IpSegmentation
  IpFunc
  SurfaceInterpolation
  GraphAlgorithms
  ImageStatistics
  ImageExtraction
  PlanarFigure
  QtWidgets
  QtWidgetsExt
)

