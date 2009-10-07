SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  cherryObjectBrowserView.cpp
  cherryObjectItem.cpp
  cherryQtObjectTableModel.cpp
)

SET(UI_FILES
  src/internal/cherryQtObjectBrowserView.ui
)

SET(MOC_H_FILES
  src/internal/cherryObjectBrowserView.h
  src/internal/cherryQtObjectTableModel.h
)

SET(RES_FILES
  resources/opencherry_ui_qt_objectinspector.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

