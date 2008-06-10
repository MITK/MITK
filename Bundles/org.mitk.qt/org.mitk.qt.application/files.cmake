SET(SRC_CPP_FILES
  mitkDataStorageEditorInput.cpp
  mitkQtApplication.cpp
  mitkStdMultiWidgetEditor.cpp
)

SET(INTERNAL_CPP_FILES
  mitkQtActionBarAdvisor.cpp
  mitkQtFileOpenAction.cpp
  mitkQtWorkbenchAdvisor.cpp
  mitkQtWorkbenchWindowAdvisor.cpp
)

SET(MOC_H_FILES
  src/internal/mitkQtFileOpenAction.h
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
