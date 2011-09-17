SET(SRC_CPP_FILES
  QmitkCloseProjectAction.cpp
  QmitkDnDFrameWidget.cpp
  QmitkDataNodeSelectionProvider.cpp
  QmitkFileOpenAction.cpp
  QmitkFileExitAction.cpp
  QmitkFunctionality.cpp
  QmitkFunctionality2.cpp
  QmitkStatusBar.cpp
  QmitkStdMultiWidgetEditor.cpp
  QmitkPreferencesDialog.cpp
  QmitkFunctionalityCoordinator.cpp
  QmitkStdMultiWidgetEditorPreferencePage.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkDataNodeSelection.cpp
  QmitkCommonActivator.cpp
  QmitkFunctionalityUtil.cpp
)

SET(MOC_H_FILES
  src/QmitkCloseProjectAction.h
  src/QmitkFileOpenAction.h
  src/QmitkFileExitAction.h
  src/QmitkPreferencesDialog.h
  src/QmitkStdMultiWidgetEditor.h
  src/QmitkDnDFrameWidget.h
  
  src/internal/QmitkCommonActivator.h

  src/QmitkStdMultiWidgetEditorPreferencePage.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

# todo: add some qt style sheet resources
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
