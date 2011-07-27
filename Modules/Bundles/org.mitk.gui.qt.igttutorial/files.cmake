SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  QmitkImageGuidedTherapyTutorialView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkImageGuidedTherapyTutorialViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkImageGuidedTherapyTutorialView.h
  src/internal/mitkPluginActivator.h
)

SET(QRC_FILES
  #resources/QmitkImageGuidedTherapyTutorialView.qrc
)

SET(CACHED_RESOURCE_FILES
  resources/ImageGuidedTherapyTutorial.png
  plugin.xml
)

SET(CPP_FILES

)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
