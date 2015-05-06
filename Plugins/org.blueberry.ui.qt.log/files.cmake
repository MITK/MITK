set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  berryLogView.cpp
  berryQtLogPlugin.cpp
  berryQtLogView.cpp
  berryQtPlatformLogModel.cpp
)

set(CPP_FILES )

set(MOC_H_FILES
  src/internal/berryLogView.h
  src/internal/berryQtLogPlugin.h
  src/internal/berryQtLogView.h
  src/internal/berryQtPlatformLogModel.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml

  resources/Logging.png
)

set(QRC_FILES
  resources/org_blueberry_ui_qt_log.qrc
)

set(UI_FILES
  src/internal/berryQtLogView.ui
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
