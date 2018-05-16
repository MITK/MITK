set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkPreprocessingResamplingView.cpp
  mitkPreprocessingResamplingActivator.cpp
)

set(UI_FILES
  src/internal/QmitkPreprocessingResamplingViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkPreprocessingResamplingView.h
  src/internal/mitkPreprocessingResamplingActivator.h
)

set(CACHED_RESOURCE_FILES
  resources/lena.xpm
  plugin.xml
)

set(QRC_FILES
  resources/QmitkPreprocessingResamplingView.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

