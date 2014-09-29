SET(CPP_FILES
  mitkIPythonService.cpp
  mitkPythonActivator.cpp
  mitkPythonService.cpp
  QmitkCtkPythonShell.cpp
  QmitkPythonVariableStackTableModel.cpp
  QmitkPythonVariableStackTableView.cpp
  QmitkPythonScriptEditorHighlighter.cpp
  QmitkPythonTextEditor.cpp
  QmitkPythonSnippets.cpp
)

if(BUILD_TESTING)
  SET(H_FILES
    Testing/mitkCommonPythonTest.h
  )
endif()

#SET(UI_FILES
#  QmitkPythonSnippets.ui
#)

SET(MOC_H_FILES
  QmitkCtkPythonShell.h
  QmitkPythonVariableStackTableModel.h
  QmitkPythonVariableStackTableView.h
  QmitkPythonScriptEditorHighlighter.h
  QmitkPythonTextEditor.h
  QmitkPythonSnippets.h
)

set(QRC_FILES
  resources/mitkPython.qrc
)
