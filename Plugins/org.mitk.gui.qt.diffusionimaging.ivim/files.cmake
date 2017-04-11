set(SRC_CPP_FILES
  QmitkIVIMWidget.cpp
  QmitkKurtosisWidget.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  
  QmitkIVIMView.cpp
  
  Perspectives/QmitkDIAppIVIMPerspective.cpp
)

set(UI_FILES
  src/internal/QmitkIVIMViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  
  src/internal/QmitkIVIMView.h

  src/internal/Perspectives/QmitkDIAppIVIMPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/IVIM_48.png
  resources/ivim.png
)

set(QRC_FILES

)


set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
