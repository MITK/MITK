set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDataManagerLightView.cpp
)

set(MOC_H_FILES
  src/internal/QmitkDataManagerLightView.h
  src/internal/mitkPluginActivator.h
)

set(CPP_FILES )

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/DataManagerLight.png
)

set(QRC_FILES
  resources/DataManagerLight.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
