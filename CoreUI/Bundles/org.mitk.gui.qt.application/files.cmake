SET(SRC_CPP_FILES
  QmitkApplication.cpp
  QmitkActionBarAdvisor.cpp
  QmitkDefaultPerspective.cpp
  QmitkWorkbenchAdvisor.cpp
  QmitkWorkbenchWindowAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  
)

SET(INTERNAL_MOC_H_FILES
)

SET(MOC_H_FILES
  
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
