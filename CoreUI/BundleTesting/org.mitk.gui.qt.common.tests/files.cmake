SET(SRC_CPP_FILES

  QmitkQtCommonTestSuite.cpp

  # API Tests
  api/QmitkFunctionalityTest.cpp
  api/QmitkMockFunctionality.cpp
  api/QmitkUiApiTestSuite.cpp
  
)

SET(INTERNAL_CPP_FILES
  
)

SET(CPP_FILES manifest.cpp)

SET(RESOURCE_FILES

#  icons/view.gif
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

