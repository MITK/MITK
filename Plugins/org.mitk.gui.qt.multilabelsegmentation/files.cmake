set(SRC_CPP_FILES
  QmitkMultiLabelSegmentationPreferencePage.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkMultiLabelSegmentationView.cpp
  QmitkThresholdAction.cpp
  QmitkAutocropAction.cpp
  QmitkConvertSurfaceToLabelAction.cpp
  QmitkConvertMaskToLabelAction.cpp
  QmitkConvertToMultiLabelSegmentationAction.cpp
  QmitkCreateMultiLabelSegmentationAction.cpp
  QmitkLoadMultiLabelPresetAction.cpp
  QmitkCreateMultiLabelPresetAction.cpp
  Common/QmitkDataSelectionWidget.cpp
  SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.cpp
  SegmentationUtilities/QmitkSegmentationUtilityWidget.cpp
  SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.cpp
  SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidget.cpp
  SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidget.cpp
  SegmentationUtilities/ImageMasking/QmitkImageMaskingWidget.cpp
  SegmentationUtilities/ConvertToMl/QmitkConvertToMlWidget.cpp
)

set(UI_FILES
  src/internal/QmitkMultiLabelSegmentationControls.ui
  src/internal/Common/QmitkDataSelectionWidgetControls.ui
  src/internal/SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesViewControls.ui
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidgetControls.ui
  src/internal/SegmentationUtilities/MorphologicalOperations/QmitkMorphologicalOperationsWidgetControls.ui
  src/internal/SegmentationUtilities/SurfaceToImage/QmitkSurfaceToImageWidgetControls.ui
  src/internal/SegmentationUtilities/ImageMasking/QmitkImageMaskingWidgetControls.ui
  src/internal/SegmentationUtilities/ConvertToMl/QmitkConvertToMlWidgetControls.ui
)

set(MOC_H_FILES
  src/QmitkMultiLabelSegmentationPreferencePage.h
  src/internal/mitkPluginActivator.h
  src/internal/QmitkMultiLabelSegmentationView.h
  src/internal/QmitkThresholdAction.h
  src/internal/QmitkAutocropAction.h
  src/internal/QmitkConvertSurfaceToLabelAction.h
  src/internal/QmitkLoadMultiLabelPresetAction.h
  src/internal/QmitkCreateMultiLabelPresetAction.h
  src/internal/QmitkConvertMaskToLabelAction.h
  src/internal/QmitkConvertToMultiLabelSegmentationAction.h
  src/internal/QmitkCreateMultiLabelSegmentationAction.h
  src/internal/Common/QmitkDataSelectionWidget.h
  src/internal/SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilityWidget.h
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.h
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
  resources/multilabelsegmentation.svg
  resources/multilabelsegmentation_utilities.svg
  resources/NewLabel_48x48.png
  resources/NewSegmentationSession_48x48.png
  resources/Opening_48x48.png
  resources/SurfaceToImage_48x48.png
  plugin.xml
)

set(QRC_FILES
  resources/multilabelsegmentation.qrc
  resources/MultiLabelSegmentationUtilities.qrc
  resources/MorphologicalOperationsWidget.qrc
  resources/BooleanOperationsWidget.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

#usFunctionEmbedResources(
#CPP_FILES
# LIBRARY_NAME "liborg_mitk_gui_qt_multilabelsegmentation"
#ROOT_DIR resources
#FILES Interactions/SegmentationInteraction.xml
#      Interactions/ConfigSegmentation.xml
#)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
