set(SRC_CPP_FILES
  QmitkSegmentationPreferencePage.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkSegmentationView.cpp
  QmitkAutocropAction.cpp
  QmitkAutocropLabelSetImageAction.cpp
  QmitkCreatePolygonModelAction.cpp
  QmitkLoadMultiLabelPresetAction.cpp
  QmitkSaveMultiLabelPresetAction.cpp
  Common/QmitkDataSelectionWidget.cpp
  Common/QmitkLabelsWidget.cpp
  Common/QmitkLayersWidget.cpp
  SegmentationUtilities/QmitkSegmentationUtilitiesView.cpp
  SegmentationUtilities/QmitkSegmentationUtilityWidget.cpp
  SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.cpp
  SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.cpp
  SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidget.cpp
  SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.cpp
  SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.cpp
)

set(UI_FILES
  src/QmitkSegmentationPreferencePageControls.ui
  src/internal/QmitkSegmentationViewControls.ui
  src/internal/Common/QmitkDataSelectionWidgetControls.ui
  src/internal/Common/QmitkLabelsWidgetControls.ui
  src/internal/Common/QmitkLayersWidgetControls.ui
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
  src/internal/QmitkAutocropAction.h
  src/internal/QmitkAutocropLabelSetImageAction.h
  src/internal/QmitkCreatePolygonModelAction.h
  src/internal/QmitkLoadMultiLabelPresetAction.h
  src/internal/QmitkSaveMultiLabelPresetAction.h
  src/internal/Common/QmitkDataSelectionWidget.h
  src/internal/Common/QmitkLabelsWidget.h
  src/internal/Common/QmitkLayersWidget.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilitiesView.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilityWidget.h
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.h
  src/internal/SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.h
  src/internal/SegmentationUtilities/ContourModelToImage/QmitkContourModelToImageWidget.h
  src/internal/SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.h
  src/internal/SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.h
)

set(CACHED_RESOURCE_FILES
  resources/segmentation.svg
  resources/segmentation_utilities.svg
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
