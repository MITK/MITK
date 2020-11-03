set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkVolumeVisualizationView.cpp
)

set(UI_FILES
  src/internal/QmitkVolumeVisualizationViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkVolumeVisualizationView.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/volume_visualization.svg
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
