SET(SRC_CPP_FILES
  QmitkDataTreeNodeSelectionProvider.cpp
  QmitkFileOpenAction.cpp
  QmitkFileExitAction.cpp
  QmitkFunctionality.cpp
  QmitkProgressBar.cpp
  QmitkStatusBar.cpp
  QmitkStdMultiWidgetEditor.cpp
  QmitkGeneralPreferencePage.cpp
  QmitkPreferencesDialog.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkDataTreeNodeSelection.cpp
  QmitkDnDFrameWidget.cpp
  QmitkStateMachineActivator.cpp
)

SET(MOC_H_FILES
  src/QmitkFileOpenAction.h
  src/QmitkFileExitAction.h
  src/QmitkProgressBar.h
  src/QmitkGeneralPreferencePage.h
  src/QmitkPreferencesDialog.h
  src/internal/QmitkDnDFrameWidget.h
)

# todo: add some qt style sheet resources
SET(RES_FILES
  resources/resources.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
