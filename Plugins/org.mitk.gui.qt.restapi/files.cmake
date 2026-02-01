set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkRestApiPreferencePage.cpp
  QmitkRestApiView.cpp
)

set(UI_FILES
  src/internal/QmitkRestApiPreferencesControls.ui
  src/internal/QmitkRestApiViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkRestApiPreferencePage.h
  src/internal/QmitkRestApiView.h
)

set(CACHED_RESOURCE_FILES
  resources/restapi.svg
  plugin.xml
)

set(QRC_FILES
  resources/restapi.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
