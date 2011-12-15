SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  videoplayer/QmitkVideoPlayer.cpp
  mitkPluginActivator.cpp
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/videoplayer/QmitkVideoPlayer.h
)

SET(CACHED_RESOURCE_FILES
  resources/videoplayer.gif
  plugin.xml
)

SET(QRC_FILES
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
