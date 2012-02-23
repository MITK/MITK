SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkImageNavigatorView.cpp
  mitkImageNavigatorPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkImageNavigatorViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkImageNavigatorPluginActivator.h
  src/internal/QmitkImageNavigatorView.h
)

SET(CACHED_RESOURCE_FILES
  resources/Slider.png
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkImageNavigatorView.qrc
)


foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

