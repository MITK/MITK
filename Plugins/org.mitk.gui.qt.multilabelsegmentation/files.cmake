set(SRC_CPP_FILES
  QmitkMultiLabelSegmentationPreferencePage.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkMultiLabelSegmentationView.cpp
  QmitkThresholdAction.cpp
  QmitkCreatePolygonModelAction.cpp
  QmitkAutocropAction.cpp
  Common/QmitkLabelSetWidget.cpp
  Common/QmitkDataSelectionWidget.cpp
  SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.cpp
  SegmentationUtilities/QmitkSegmentationUtilityWidget.cpp
  SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.cpp
)

set(UI_FILES
  src/internal/QmitkMultiLabelSegmentationControls.ui
  src/internal/Common/QmitkLabelSetWidgetControls.ui
  src/internal/Common/QmitkDataSelectionWidgetControls.ui
  src/internal/SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesViewControls.ui
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidgetControls.ui
)

set(MOC_H_FILES
  src/QmitkMultiLabelSegmentationPreferencePage.h
  src/internal/mitkPluginActivator.h
  src/internal/QmitkMultiLabelSegmentationView.h
  src/internal/QmitkThresholdAction.h
  src/internal/QmitkCreatePolygonModelAction.h
  src/internal/QmitkAutocropAction.h
  src/internal/Common/QmitkLabelSetWidget.h
  src/internal/Common/QmitkDataSelectionWidget.h
  src/internal/SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.h
  src/internal/SegmentationUtilities/QmitkSegmentationUtilityWidget.h
  src/internal/SegmentationUtilities/BooleanOperations/QmitkBooleanOperationsWidget.h
)

set(CACHED_RESOURCE_FILES
  resources/multilabelsegmentation.png
  resources/MultiLabelSegmentationUtilities_48x48.png
  plugin.xml
)

set(QRC_FILES
  resources/multilabelsegmentation.qrc
  resources/MultiLabelSegmentationUtilities.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
