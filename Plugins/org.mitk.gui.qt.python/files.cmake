set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkPythonView.cpp
)

set(MOC_H_FILES
  src/internal/QmitkPythonView.h
  src/internal/mitkPluginActivator.h
)

set(CPP_FILES )

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/Python.png
)

set(QRC_FILES
  resources/Python.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
