SET(SRC_CPP_FILES
  QmitkCloseProjectAction.cpp
  QmitkDefaultDropTargetListener.cpp
  QmitkFileExitAction.cpp
  QmitkFileOpenAction.cpp
  QmitkPreferencesDialog.cpp
  QmitkStatusBar.cpp
)

SET(INTERNAL_CPP_FILES
  org_mitk_gui_qt_application_Activator.cpp
  QmitkEditorsPreferencePage.cpp
  QmitkGeneralPreferencePage.cpp
)

SET(MOC_H_FILES
  src/QmitkCloseProjectAction.h
  src/QmitkFileExitAction.h
  src/QmitkFileOpenAction.h
  src/QmitkPreferencesDialog.h

  src/internal/org_mitk_gui_qt_application_Activator.h
  src/internal/QmitkEditorsPreferencePage.h
  src/internal/QmitkGeneralPreferencePage.h
)

SET(UI_FILES
  src/QmitkPreferencesDialog.ui
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(QRC_FILES
  resources/resources.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
