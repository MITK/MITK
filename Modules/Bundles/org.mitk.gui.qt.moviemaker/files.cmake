SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkMovieMaker.cpp
  mitkMovieMakerPluginActivator.cpp

)

SET(UI_FILES
  src/internal/QmitkMovieMakerControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkMovieMakerPluginActivator.h
  src/internal/QmitkMovieMaker.h
)

SET(CACHED_RESOURCE_FILES
  resources/icon.xpm
  plugin.xml
)

SET(RES_FILES
 resources/QmitkMovieMaker.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

