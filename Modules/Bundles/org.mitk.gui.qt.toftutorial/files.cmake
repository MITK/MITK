SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkToFTutorialView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkToFTutorialViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkToFTutorialView.h
  src/internal/mitkPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/icon.xpm
)

SET(QRC_FILES
  resources/QmitkToFTutorialView.qrc
)

SET(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

