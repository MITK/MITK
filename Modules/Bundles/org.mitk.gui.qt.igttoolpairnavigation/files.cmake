SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkToolPairNavigationPluginActivator.cpp
  QmitkToolPairNavigationView.cpp
)

SET(UI_FILES
  src/internal/QmitkToolPairNavigationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkToolPairNavigationView.h
  src/internal/mitkToolPairNavigationPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/toolpair.png
)

SET(QRC_FILES
  resources/QmitkToolPairNavigationView.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

