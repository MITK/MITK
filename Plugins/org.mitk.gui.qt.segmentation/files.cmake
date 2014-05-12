set(SRC_CPP_FILES
  QmitkSegmentationPreferencePage.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkSegmentationView.cpp
  QmitkThresholdAction.cpp
  QmitkCreatePolygonModelAction.cpp
  #QmitkStatisticsAction.cpp
  QmitkAutocropAction.cpp
  QmitkDeformableClippingPlaneView.cpp
  Common/QmitkDataSelectionWidget.cpp
  SegmentationUtilities/QmitkSegmentationUtilitiesView.cpp
  SegmentationUtilities/QmitkSegmentationUtilityWidget.cpp
  SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.cpp
  SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.cpp
  SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidget.cpp
  SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.cpp
  SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.cpp
  SegmentationUtilities/CTKWidgets/QmitkCTKWidgetsWidget.cpp
)

set(UI_FILES
  src/internal/QmitkSegmentationControls.ui
  src/internal/QmitkDeformableClippingPlaneViewControls.ui
  src/internal/Common/QmitkDataSelectionWidgetControls.ui
  src/internal/SegmentationUtilities/QmitkSegmentationUtilitiesViewControls.ui
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidgetControls.ui
  src/internal/SegmentationUtilities/ImageMasking/QmitkImageMaskingWidgetControls.ui
  src/internal/SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidgetControls.ui
  src/internal/SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidgetControls.ui
  src/internal/SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidgetControls.ui
)

set(MOC_H_FILES
  src/QmitkSegmentationPreferencePage.h
  src/internal/mitkPluginActivator.h
  src/internal/QmitkSegmentationView.h
  src/internal/QmitkThresholdAction.h
  src/internal/QmitkCreatePolygonModelAction.h
  #src/internal/QmitkStatisticsAction.h
  src/internal/QmitkAutocropAction.h
  src/internal/QmitkDeformableClippingPlaneView.h
  src/internal/Common/QmitkDataSelectionWidget.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilitiesView.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilityWidget.h
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.h
  src/internal/SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.h
  src/internal/SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidget.h
  src/internal/SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.h
  src/internal/SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.h
  src/internal/SegmentationUtilities/CTKWidgets/QmitkCTKWidgetsWidget.h
)

set(CACHED_RESOURCE_FILES
  resources/segmentation.png
  resources/deformablePlane.png
  resources/clipping_plane_translate_48x48.png
  resources/clipping_plane_rotate48x48.png
  resources/clipping_plane_deform48x48.png
  resources/SegmentationUtilities_48x48.png
  plugin.xml
)

set(QRC_FILES
  resources/segmentation.qrc
  resources/SegmentationUtilities.qrc
  resources/BooleanOperationsWidget.qrc
  resources/MorphologicalOperationsWidget.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
