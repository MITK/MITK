SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkNavigationDataPlayerPluginActivator.cpp
  QmitkNavigationDataPlayerView.cpp
)

SET(UI_FILES
  src/internal/QmitkNavigationDataPlayerViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkNavigationDataPlayerView.h
  src/internal/mitkNavigationDataPlayerPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/icon.png
)

SET(QRC_FILES
  resources/QmitkNavigationDataPlayerView.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

