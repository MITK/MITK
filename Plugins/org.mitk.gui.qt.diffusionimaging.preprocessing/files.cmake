set(SRC_CPP_FILES


)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  
  QmitkPreprocessingView.cpp
)

set(UI_FILES
  src/internal/QmitkPreprocessingViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h

  src/internal/QmitkPreprocessingView.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/preprocessing.png
  resources/dwi2.png
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
