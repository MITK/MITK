set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkRenderWindowManagerView.cpp
	QmitkLayerManagerAddNodeWidget.cpp
)

set(UI_FILES
  src/internal/QmitkRenderWindowManagerControls.ui
	src/internal/QmitkLayerManagerAddNodeControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkRenderWindowManagerView.h
  src/internal/QmitkLayerManagerAddNodeWidget.h
)

set(CACHED_RESOURCE_FILES
	resources/LayerManager_48.png
  plugin.xml
)

set(QRC_FILES 
	resources/renderwindowmanager.qrc
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
