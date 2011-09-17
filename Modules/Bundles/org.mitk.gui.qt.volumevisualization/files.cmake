SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkVolumeVisualizationView.cpp
)

SET(UI_FILES
  src/internal/QmitkVolumeVisualizationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkVolumeVisualizationView.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/icon.xpm
)

SET(QRC_FILES

)


SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
