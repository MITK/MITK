SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  QmitkMovieMaker.cpp
  mitkMovieMakerPluginActivator.cpp
  QmitkScreenshotMaker.cpp

)

SET(UI_FILES
  src/internal/QmitkMovieMakerControls.ui
  src/internal/QmitkScreenshotMakerControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkMovieMakerPluginActivator.h
  src/internal/QmitkMovieMaker.h
  src/internal/QmitkScreenshotMaker.h
)

SET(CACHED_RESOURCE_FILES
  resources/icon.xpm
  plugin.xml
 resources/play.xpm
 resources/stop.xpm
 resources/pause.xpm
  resources/screenshot_maker.png
)

SET(RES_FILES
 resources/QmitkMovieMaker.qrc
 resources/play.xpm
 resources/stop.xpm
 resources/pause.xpm
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

