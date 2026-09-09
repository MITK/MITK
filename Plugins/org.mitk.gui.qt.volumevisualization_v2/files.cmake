set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkVolumeVisualizationV2View.cpp
)

set(UI_FILES
  src/internal/QmitkVolumeVisualizationV2View.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkVolumeVisualizationV2View.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/volume_visualization.svg
)

set(QRC_FILES
  resources/volumevisualization_v2.qrc
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
