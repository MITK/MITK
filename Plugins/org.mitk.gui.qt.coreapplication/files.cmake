set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_coreapplication_Activator.cpp
  QmitkActionBarAdvisor.cpp
  QmitkCoreApplication.cpp
  QmitkDefaultPerspective.cpp
  QmitkWorkbenchAdvisor.cpp
  QmitkWorkbenchWindowAdvisor.cpp
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_coreapplication_Activator.h

  src/internal/QmitkCoreApplication.h
  src/internal/QmitkDefaultPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
