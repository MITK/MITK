SET(SRC_CPP_FILES
  QmitkApplication.cpp
  QmitkStdMultiWidgetEditor.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkActionBarAdvisor.cpp
  QmitkFileOpenAction.cpp
  QmitkWorkbenchAdvisor.cpp
  QmitkWorkbenchWindowAdvisor.cpp
)

SET(MOC_H_FILES
  src/internal/QmitkFileOpenAction.h
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
