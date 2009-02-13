SET(SRC_CPP_FILES
  QmitkIsoSurface.cpp
  #QmitkIsoSurfaceControls.cpp
)

SET(INTERNAL_CPP_FILES

)

SET(UI_FILES
  src/QmitkIsoSurfaceControls.ui
)

SET(MOC_H_FILES
  src/QmitkIsoSurface.h
  #src/QmitkIsoSurfaceControls.h
)

SET(RES_FILES
	resources.qrc
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})