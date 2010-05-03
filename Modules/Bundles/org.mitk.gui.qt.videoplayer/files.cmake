SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkVideoPlayer.cpp
)

SET(MOC_H_FILES
  src/internal/QmitkVideoPlayer.h
)

SET(RESOURCE_FILES
  resources/videoplayer.gif
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
