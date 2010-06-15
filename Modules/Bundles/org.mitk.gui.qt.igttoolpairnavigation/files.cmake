SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkToolPairNavigationView.cpp

)

SET(UI_FILES
  src/internal/QmitkToolPairNavigationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkToolPairNavigationView.h
)

SET(RESOURCE_FILES
  resources/toolpair.png
)

SET(RES_FILES
  resources/QmitkToolPairNavigationView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

