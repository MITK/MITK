SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkToFUtilView.cpp
  QmitkToFImageBackground.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkToFUtilViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkToFUtilView.h
  src/internal/QmitkToFImageBackground.h
  src/internal/mitkPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/icon.xpm
)

SET(QRC_FILES
  resources/QmitkToFUtilView.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

