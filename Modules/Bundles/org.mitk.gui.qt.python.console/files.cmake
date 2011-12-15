SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkPythonConsoleView.cpp
  QmitkPythonEditor.cpp
  QmitkPythonScriptEditor.cpp
  QmitkPythonVariableStack.cpp
  QmitkPythonCommandHistory.cpp
  QmitkPythonScriptEditorHighlighter.cpp
  QmitkPythonCommandHistoryTreeWidget.cpp
  QmitkPythonVariableStackTreeWidget.cpp
  QmitkPythonTextEditor.cpp
  QmitkCTKPythonShell.cpp
  QmitkPythonMediator.cpp
  QmitkPythonPerspective.cpp
  QmitkPythonSnippets.cpp
)

SET(UI_FILES
  src/internal/QmitkPythonConsoleViewControls.ui
  src/internal/QmitkPythonEditor.ui
  src/internal/QmitkPythonVariableStack.ui
  src/internal/QmitkPythonSnippets.ui
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkPythonCommandHistory.h
  src/internal/QmitkPythonCommandHistoryTreeWidget.h
  src/internal/QmitkPythonConsoleView.h
  src/internal/QmitkPythonEditor.h
  src/internal/QmitkPythonPerspective.h
  src/internal/QmitkPythonScriptEditor.h
  src/internal/QmitkPythonScriptEditorHighlighter.h
  src/internal/QmitkPythonTextEditor.h
  src/internal/QmitkPythonVariableStack.h
  src/internal/QmitkPythonSnippets.h
  src/internal/QmitkPythonVariableStackTreeWidget.h
  src/internal/QmitkCTKPythonShell.h
)

SET(CACHED_RESOURCE_FILES
  resources/py.png
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkPythonConsoleView.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

