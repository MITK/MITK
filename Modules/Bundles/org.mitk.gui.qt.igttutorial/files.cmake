SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkImageGuidedTherapyTutorialView.cpp

)

SET(UI_FILES
  src/internal/QmitkImageGuidedTherapyTutorialViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkImageGuidedTherapyTutorialView.h
)

SET(RES_FILES
  #resources/QmitkImageGuidedTherapyTutorialView.qrc
)

SET(RESOURCE_FILES
  resources/ImageGuidedTherapy.png
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

