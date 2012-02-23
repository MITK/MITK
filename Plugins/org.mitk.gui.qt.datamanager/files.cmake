SET(SRC_CPP_FILES
  berrySingleNodeSelection.cpp
  QmitkDataManagerView.cpp 
  QmitkDataManagerPreferencePage.cpp
  QmitkDataManagerHotkeysPrefPage.cpp
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkPropertyListView.cpp
  QmitkNodeTableViewKeyFilter.cpp 
  QmitkInfoDialog.cpp 
)

SET(MOC_H_FILES
  src/QmitkDataManagerView.h
  src/QmitkDataManagerPreferencePage.h
  src/QmitkDataManagerHotkeysPrefPage.h
  src/internal/QmitkNodeTableViewKeyFilter.h
  src/internal/QmitkPropertyListView.h
  src/internal/QmitkInfoDialog.h
  src/internal/mitkPluginActivator.h
)

SET(CPP_FILES )

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/DataManager_48.png
  resources/propertylist.png
)

SET(QRC_FILES
  resources/datamanager.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
