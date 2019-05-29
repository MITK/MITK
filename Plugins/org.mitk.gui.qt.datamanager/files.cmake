set(SRC_CPP_FILES
  berrySingleNodeSelection.cpp
  QmitkDataManagerHotkeysPrefPage.cpp
  QmitkDataManagerPreferencePage.cpp
  QmitkDataManagerView.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDataManagerItemDelegate.cpp
  QmitkNodeTableViewKeyFilter.cpp
)

set(MOC_H_FILES
  src/QmitkDataManagerHotkeysPrefPage.h
  src/QmitkDataManagerPreferencePage.h
  src/QmitkDataManagerView.h
  src/internal/QmitkDataManagerItemDelegate.h
  src/internal/QmitkNodeTableViewKeyFilter.h
  src/internal/mitkPluginActivator.h
)

set(CPP_FILES )

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/data-manager.svg
)

set(QRC_FILES
  resources/datamanager.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
