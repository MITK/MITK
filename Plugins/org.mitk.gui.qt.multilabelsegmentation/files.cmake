set(SRC_CPP_FILES
  QmitkMultiLabelSegmentationPreferencePage.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkMultiLabelSegmentationView.cpp
  Common/QmitkLabelSetWidget.cpp

)

set(UI_FILES
  src/internal/QmitkMultiLabelSegmentationControls.ui
  src/internal/Common/QmitkLabelSetWidgetControls.ui
)

set(MOC_H_FILES
  src/QmitkMultiLabelSegmentationPreferencePage.h
  src/internal/mitkPluginActivator.h
  src/internal/QmitkMultiLabelSegmentationView.h
  src/internal/Common/QmitkLabelSetWidget.h

)

set(CACHED_RESOURCE_FILES
  resources/multilabelsegmentation.png
  plugin.xml
)

set(QRC_FILES
  resources/multilabelsegmentation.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
