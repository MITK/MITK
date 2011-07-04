SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkViewInitializationView.cpp
)

SET(UI_FILES
  src/internal/QmitkViewInitializationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkViewInitializationView.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/viewInitializationIcon.xpm
)

SET(QRC_FILES

)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

