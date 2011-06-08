SET(SRC_CPP_FILES
  QmitkApplication.cpp
  QmitkActionBarAdvisor.cpp
  QmitkDefaultPerspective.cpp
  QmitkWorkbenchAdvisor.cpp
  QmitkWorkbenchWindowAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
)

SET(MOC_H_FILES
  src/QmitkApplication.h
  src/QmitkDefaultPerspective.h

  src/internal/mitkPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
