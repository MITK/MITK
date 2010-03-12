SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkMeasurement.cpp
  QmitkPlanarFiguresTableModel.cpp
)

SET(UI_FILES
)

SET(MOC_H_FILES
  src/internal/QmitkMeasurement.h
  src/internal/QmitkPlanarFiguresTableModel.h
)

SET(RESOURCE_FILES
  resources/measurement.png
  resources/angle.png
  resources/arrow.png
  resources/circle.png
  resources/four-point-angle.png
  resources/line.png
  resources/measurement.png
  resources/path.png
  resources/polygon.png
  resources/rectangle.png
  resources/text.png
)

SET(RES_FILES
  resources/measurement.qrc
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
