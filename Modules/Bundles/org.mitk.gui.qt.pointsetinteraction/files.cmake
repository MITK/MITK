SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkPointSetInteractionView.cpp

)

SET(UI_FILES
  src/internal/QmitkPointSetInteractionViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkPointSetInteractionView.h
)

SET(RESOURCE_FILES
  resources/pointset_interaction.png
)

SET(RES_FILES
  resources/QmitkPointSetInteractionView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

