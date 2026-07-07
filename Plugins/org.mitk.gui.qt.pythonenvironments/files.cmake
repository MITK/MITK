set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkPythonEnvironmentsView.cpp
)

set(UI_FILES
  src/internal/QmitkPythonEnvironmentsView.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkPythonEnvironmentsView.h
)

set(CACHED_RESOURCE_FILES
  resources/python-gear.svg
  plugin.xml
)

set(QRC_FILES
  resources/QmitkPythonEnvironments.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
