set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkThresholdAction.cpp
  QmitkConvertSurfaceToLabelAction.cpp
  QmitkConvertMaskToLabelAction.cpp
  QmitkConvertToMultiLabelSegmentationAction.cpp
  QmitkCreateMultiLabelSegmentationAction.cpp
  Common/QmitkDataSelectionWidget.cpp
  SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.cpp
  SegmentationUtilities/QmitkSegmentationUtilityWidget.cpp
  SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.cpp
  SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidget.cpp
  SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.cpp
  SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.cpp
  SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.cpp
  SegmentationUtilities/ConvertToMl/QmitkConvertToMlWidget.cpp
)

set(UI_FILES
  src/internal/Common/QmitkDataSelectionWidgetControls.ui
  src/internal/SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesViewControls.ui
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidgetControls.ui
  src/internal/SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidgetControls.ui
  src/internal/SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidgetControls.ui
  src/internal/SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidgetControls.ui
  src/internal/SegmentationUtilities/ImageMasking/QmitkImageMaskingWidgetControls.ui
  src/internal/SegmentationUtilities/ConvertToMl/QmitkConvertToMlWidgetControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkThresholdAction.h
  src/internal/QmitkConvertSurfaceToLabelAction.h
  src/internal/QmitkConvertMaskToLabelAction.h
  src/internal/QmitkConvertToMultiLabelSegmentationAction.h
  src/internal/QmitkCreateMultiLabelSegmentationAction.h
  src/internal/Common/QmitkDataSelectionWidget.h
  src/internal/SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilityWidget.h
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.h
  src/internal/SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidget.h
  src/internal/SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.h
  src/internal/SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.h
  src/internal/SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.h
  src/internal/SegmentationUtilities/ConvertToMl/QmitkConvertToMlWidget.h
)

set(CACHED_RESOURCE_FILES
  resources/BooleanDifference_48x48.png
  resources/BooleanIntersection_48x48.png
  resources/BooleanOperations_48x48.png
  resources/BooleanUnion_48x48.png
  resources/Closing_48x48.png
  resources/CTKWidgets_48x48.png
  resources/deformablePlane.png
  resources/Dilate_48x48.png
  resources/Erode_48x48.png
  resources/FillHoles_48x48.png
  resources/Icons.svg
  resources/ImageMasking_48x48.png
  resources/MorphologicalOperations_48x48.png
  resources/multilabelsegmentation_utilities.svg
  resources/Opening_48x48.png
  resources/SurfaceToImage_48x48.png
  plugin.xml
)

set(QRC_FILES
  resources/MultiLabelSegmentationUtilities.qrc
  resources/MorphologicalOperationsWidget.qrc
  resources/BooleanOperationsWidget.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
