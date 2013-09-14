set(SRC_CPP_FILES
  berrySingleNodeSelection.cpp
  QmitkDataManagerView.cpp
  QmitkDataManagerPreferencePage.cpp
  QmitkDataManagerHotkeysPrefPage.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkNodeTableViewKeyFilter.cpp
  QmitkInfoDialog.cpp
)

set(MOC_H_FILES
  src/QmitkDataManagerView.h
  src/QmitkDataManagerPreferencePage.h
  src/QmitkDataManagerHotkeysPrefPage.h
  src/internal/QmitkNodeTableViewKeyFilter.h
  src/internal/QmitkInfoDialog.h
  src/internal/mitkPluginActivator.h
)

set(CPP_FILES )

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/DataManager_48.png
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
