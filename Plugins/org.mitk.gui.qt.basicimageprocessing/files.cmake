SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkBasicImageProcessingView.cpp
  mitkBasicImageProcessingActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkBasicImageProcessingViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkBasicImageProcessingView.h
  src/internal/mitkBasicImageProcessingActivator.h
)

SET(CACHED_RESOURCE_FILES
  resources/lena.xpm
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkBasicImageProcessingView.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

