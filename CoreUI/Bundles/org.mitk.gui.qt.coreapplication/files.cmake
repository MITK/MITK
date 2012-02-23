SET(SRC_CPP_FILES
)

SET(INTERNAL_CPP_FILES
  org_mitk_gui_qt_coreapplication_Activator.cpp
  QmitkActionBarAdvisor.cpp
  QmitkCoreApplication.cpp
  QmitkDefaultPerspective.cpp
  QmitkWorkbenchAdvisor.cpp
  QmitkWorkbenchWindowAdvisor.cpp
)

SET(MOC_H_FILES
  src/internal/org_mitk_gui_qt_coreapplication_Activator.h

  src/internal/QmitkCoreApplication.h
  src/internal/QmitkDefaultPerspective.h
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
