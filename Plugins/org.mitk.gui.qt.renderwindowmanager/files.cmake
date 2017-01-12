set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkRenderWindowManagerView.cpp
)

set(UI_FILES
  src/internal/QmitkRenderWindowManagerControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkRenderWindowManagerView.h
)

set(CACHED_RESOURCE_FILES
	resources/LayerManager_48.png
  plugin.xml
)

set(QRC_FILES 
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
