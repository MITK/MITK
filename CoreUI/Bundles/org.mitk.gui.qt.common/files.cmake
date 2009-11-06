SET(SRC_CPP_FILES
  QmitkDnDFrameWidget.cpp
  QmitkDataTreeNodeSelectionProvider.cpp
  QmitkFileOpenAction.cpp
  QmitkFileSaveProjectAction.cpp
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
  QmitkCommonActivator.cpp
)

SET(MOC_H_FILES
  src/QmitkFileOpenAction.h
  src/QmitkFileSaveProjectAction.h
  src/QmitkFileExitAction.h
  src/QmitkProgressBar.h
  src/QmitkPreferencesDialog.h
  src/QmitkDnDFrameWidget.h
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
