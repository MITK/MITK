SET(SRC_CPP_FILES
  cherryUITestApplication.cpp
  
  harness/cherryUITestCase.cpp
  
  util/cherryEmptyPerspective.cpp
)

SET(INTERNAL_CPP_FILES
  cherryUITestWorkbenchAdvisor.cpp
  cherryUITestWorkbenchWindowAdvisor.cpp
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

