SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkColourImageProcessingView.cpp
  mitkColourImageProcessor.cpp
  mitkColourImageProcessingActivator.cpp

)

SET(UI_FILES
  src/internal/QmitkColourImageProcessingViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkColourImageProcessingView.h
  src/internal/mitkColourImageProcessingActivator.h
  
)

SET(CACHED_RESOURCE_FILES
  resources/ColorImageProcessing.png
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkColourImageProcessingView.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

