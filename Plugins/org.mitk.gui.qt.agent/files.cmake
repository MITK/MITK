set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkAgentPluginActivator.cpp
  QmitkAgentPreferencePage.cpp
  QmitkAgentView.cpp
)

set(MOC_H_FILES
  src/internal/QmitkAgentPluginActivator.h
  src/internal/QmitkAgentPreferencePage.h
  src/internal/QmitkAgentView.h
)

set(CACHED_RESOURCE_FILES
  resources/Agent.svg
  plugin.xml
)

set(QRC_FILES
  resources/Agent.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()
