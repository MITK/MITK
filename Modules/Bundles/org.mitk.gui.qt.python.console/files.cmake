SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
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
)

SET(UI_FILES
  src/internal/QmitkPythonConsoleViewControls.ui
  src/internal/QmitkPythonEditor.ui
  src/internal/QmitkPythonVariableStack.ui
)

SET(MOC_H_FILES
  src/internal/QmitkPythonCommandHistory.h
  src/internal/QmitkPythonCommandHistoryTreeWidget.h
  src/internal/QmitkPythonConsoleView.h
  src/internal/QmitkPythonScriptEditor.h
  src/internal/QmitkPythonScriptEditorHighlighter.h
  src/internal/QmitkPythonTextEditor.h
  src/internal/QmitkPythonVariableStack.h
  src/internal/QmitkPythonVariableStackTreeWidget.h
)

SET(RESOURCE_FILES
  resources/py.png
)

SET(RES_FILES
  resources/QmitkPythonConsoleView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

