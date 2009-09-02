SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkVolumeVisualizationView.cpp
)

SET(UI_FILES
  src/internal/QmitkVolumeVisualizationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkVolumeVisualizationView.h
)

SET(RESOURCE_FILES
  resources/icon.xpm
)

SET(RES_FILES
  resources/QmitkVolumeVisualizationView.qrc
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
