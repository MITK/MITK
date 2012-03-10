SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  berryLogView.cpp
  berryQtLogPlugin.cpp
  berryQtLogView.cpp
  berryQtPlatformLogModel.cpp
)

SET(CPP_FILES )

SET(MOC_H_FILES
  src/internal/berryLogView.h
  src/internal/berryQtLogPlugin.h
  src/internal/berryQtLogView.h
  src/internal/berryQtPlatformLogModel.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/Logging.png
)

SET(QRC_FILES
  resources/org_blueberry_ui_qt_log.qrc
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
