SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  QmitkMITKIGTNavigationToolManagerView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkMITKIGTNavigationToolManagerViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkMITKIGTNavigationToolManagerView.h
  src/internal/mitkPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  resources/icon.xpm
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkMITKIGTNavigationToolManagerView.qrc
)

SET(CPP_FILES

)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})