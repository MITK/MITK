SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  berryLogView.cpp
  berryQtLogPlugin.cpp
  berryQtLogView.cpp
  berryQtPlatformLogModel.cpp
)

SET(CPP_FILES manifest.cpp)

SET(MOC_H_FILES
  src/internal/berryQtLogView.h
  src/internal/berryQtPlatformLogModel.h
)

SET(RESOURCE_FILES
  resources/Logging.png
)

SET(UI_FILES
  src/internal/berryQtLogView.ui
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})