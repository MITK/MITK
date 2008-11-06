SET(SRC_CPP_FILES
  QmitkVolumeRenderingView.cpp
)

SET(MOC_H_FILES
  src/QmitkVolumeRenderingView.h
)

SET(INTERNAL_CPP_FILES
 
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
