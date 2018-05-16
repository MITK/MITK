set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  mitkLaserControlPluginActivator.cpp
  QmitkLaserControl.cpp

)

set(UI_FILES
  src/internal/QmitkLaserControlControls.ui
)

set(MOC_H_FILES
  src/internal/mitkLaserControlPluginActivator.h
  src/internal/QmitkLaserControl.h
)

set(CACHED_RESOURCE_FILES
  resources/iconLaserControl.svg
  plugin.xml
)

set(QRC_FILES

)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
