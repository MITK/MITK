SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  cherryLogView.cpp
  cherryQtLogPlugin.cpp
  cherryQtLogView.cpp
  cherryQtPlatformLogModel.cpp
)

SET(CPP_FILES manifest.cpp)

SET(MOC_H_FILES
  src/internal/cherryQtLogView.h
  src/internal/cherryQtPlatformLogModel.h
)

SET(RESOURCE_FILES
  resources/LogView.png
)

SET(UI_FILES
  src/internal/cherryQtLogView.ui
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})