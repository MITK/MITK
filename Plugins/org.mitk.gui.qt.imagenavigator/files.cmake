set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkImageNavigatorView.cpp
  mitkImageNavigatorPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkImageNavigatorViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkImageNavigatorPluginActivator.h
  src/internal/QmitkImageNavigatorView.h
)

set(CACHED_RESOURCE_FILES
  resources/Slider.png
  plugin.xml
)

set(QRC_FILES
  resources/QmitkImageNavigatorView.qrc
)


foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

