SET(SRC_CPP_FILES
  QmitkInteractiveSegmentationView.cpp
)

SET(INTERNAL_CPP_FILES
 
)

SET(UI_FILES
  src/QmitkInteractiveSegmentationControls.ui
)

SET(MOC_H_FILES
  src/QmitkInteractiveSegmentationView.h
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
