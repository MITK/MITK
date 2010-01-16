SET(SRC_CPP_FILES
  berrySingleNodeSelection.cpp
  QmitkDataManagerView.cpp 
  QmitkDataManagerPreferencePage.cpp
  QmitkDataManagerHotkeysPrefPage.cpp
  QmitkHotkeyLineEdit.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkPropertyListView.cpp
  QmitkNodeTableViewKeyFilter.cpp 
  QmitkInfoDialog.cpp 
)

SET(MOC_H_FILES
  src/QmitkDataManagerView.h
  src/QmitkHotkeyLineEdit.h
  src/internal/QmitkNodeTableViewKeyFilter.h
  src/internal/QmitkInfoDialog.h
)

SET(CPP_FILES manifest.cpp)

SET(RESOURCE_FILES
  resources/DataManager_48.png
  resources/propertylist.png
)

SET(RES_FILES
  resources/datamanager.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
