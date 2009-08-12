SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkViewInitializationView.cpp

)

SET(UI_FILES
  src/internal/QmitkViewInitializationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkViewInitializationView.h
)

SET(RESOURCE_FILES
  resources/viewInitializationIcon.xpm
)

SET(RES_FILES
  resources/QmitkViewInitializationView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

