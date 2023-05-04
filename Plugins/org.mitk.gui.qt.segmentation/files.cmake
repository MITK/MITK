set(SRC_CPP_FILES
  QmitkSegmentationPreferencePage.cpp
  QmitkNewSegmentationDialog.cpp
  QmitkLabelSetWidget.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkSegmentationView.cpp
  QmitkSegmentationUtilitiesView.cpp
  QmitkSegmentationTaskListView.cpp
  QmitkAutocropAction.cpp
  QmitkAutocropLabelSetImageAction.cpp
  QmitkCreatePolygonModelAction.cpp
  QmitkLoadMultiLabelPresetAction.cpp
  QmitkSaveMultiLabelPresetAction.cpp
  QmitkConvertSurfaceToLabelAction.cpp
  QmitkConvertMaskToLabelAction.cpp
  QmitkConvertToMultiLabelSegmentationAction.cpp
  QmitkCreateMultiLabelSegmentationAction.cpp
  Common/QmitkLabelsWidget.cpp
  Common/QmitkLayersWidget.cpp
)

set(UI_FILES
  src/QmitkSegmentationPreferencePageControls.ui
  src/QmitkNewSegmentationDialog.ui
  src/QmitkLabelSetWidgetControls.ui
  src/internal/QmitkSegmentationViewControls.ui
  src/internal/QmitkSegmentationUtilitiesViewControls.ui
  src/internal/QmitkSegmentationTaskListView.ui
  src/internal/Common/QmitkLabelsWidgetControls.ui
  src/internal/Common/QmitkLayersWidgetControls.ui
)

set(MOC_H_FILES
  src/QmitkSegmentationPreferencePage.h
  src/QmitkNewSegmentationDialog.h
  src/QmitkLabelSetWidget.h
  src/internal/mitkPluginActivator.h
  src/internal/QmitkSegmentationView.h
  src/internal/QmitkSegmentationUtilitiesView.h
  src/internal/QmitkSegmentationTaskListView.h
  src/internal/QmitkAutocropAction.h
  src/internal/QmitkAutocropLabelSetImageAction.h
  src/internal/QmitkCreatePolygonModelAction.h
  src/internal/QmitkLoadMultiLabelPresetAction.h
  src/internal/QmitkSaveMultiLabelPresetAction.h
  src/internal/QmitkConvertSurfaceToLabelAction.h
  src/internal/QmitkConvertMaskToLabelAction.h
  src/internal/QmitkConvertToMultiLabelSegmentationAction.h
  src/internal/QmitkCreateMultiLabelSegmentationAction.h
  src/internal/Common/QmitkLabelsWidget.h
  src/internal/Common/QmitkLayersWidget.h
)

set(CACHED_RESOURCE_FILES
  resources/segmentation.svg
  resources/segmentation_utilities.svg
  resources/SegmentationTaskListIcon.svg
  plugin.xml
)

set(QRC_FILES
  resources/segmentation.qrc
  resources/SegmentationUtilities.qrc
  resources/SegmentationTaskList.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
