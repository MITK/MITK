SET(SRC_CPP_FILES
)

SET(INTERNAL_CPP_FILES
  QmitkIsoSurface.cpp
)

SET(UI_FILES
  src/internal/QmitkIsoSurfaceControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkIsoSurface.h
)

SET(RES_FILES
  resources/isosurface.qrc
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
