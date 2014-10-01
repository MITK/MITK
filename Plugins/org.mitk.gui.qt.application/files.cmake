set(SRC_CPP_FILES
  QmitkCloseProjectAction.cpp
  QmitkDefaultDropTargetListener.cpp
  QmitkFileExitAction.cpp
  QmitkFileOpenAction.cpp
  QmitkFileSaveAction.cpp
  QmitkPreferencesDialog.cpp
  QmitkStatusBar.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_application_Activator.cpp
  QmitkEditorsPreferencePage.cpp
  QmitkGeneralPreferencePage.cpp
)

set(MOC_H_FILES
  src/QmitkCloseProjectAction.h
  src/QmitkFileExitAction.h
  src/QmitkFileOpenAction.h
  src/QmitkFileSaveAction.h
  src/QmitkPreferencesDialog.h

  src/internal/org_mitk_gui_qt_application_Activator.h
  src/internal/QmitkEditorsPreferencePage.h
  src/internal/QmitkGeneralPreferencePage.h
)

set(UI_FILES
  src/QmitkPreferencesDialog.ui
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(QRC_FILES
  resources/resources.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
