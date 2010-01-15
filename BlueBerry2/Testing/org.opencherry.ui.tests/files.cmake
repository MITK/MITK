SET(SRC_CPP_FILES

  cherryUiTestSuite.cpp

  # API Tests
  api/cherryIPageLayoutTest.cpp
  api/cherryIViewPartTest.cpp
  api/cherryIWorkbenchPartTest.cpp
  api/cherryMockSelectionProvider.cpp
  api/cherryMockViewPart.cpp
  api/cherryMockWorkbenchPart.cpp
  api/cherrySaveableMockViewPart.cpp
  api/cherryUiApiTestSuite.cpp
  api/cherryXMLMementoTest.cpp
  
  
  # Utility classes
  util/cherryCallHistory.cpp
)

SET(INTERNAL_CPP_FILES
  
)

SET(CPP_FILES manifest.cpp)

SET(RESOURCE_FILES

  icons/view.gif
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

