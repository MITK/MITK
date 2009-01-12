SET(SRC_CPP_FILES
  QmitkMeasurement.cpp
  #QmitkMeasurementControls.cpp
)

SET(INTERNAL_CPP_FILES
 
)

SET(UI_FILES
  src/QmitkMeasurementControls.ui
)

SET(MOC_H_FILES
  src/QmitkMeasurement.h
  #src/QmitkMeasurementControls.h
)

SET(RES_FILES
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
