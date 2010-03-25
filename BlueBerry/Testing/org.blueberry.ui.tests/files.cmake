SET(SRC_CPP_FILES

  berryUiTestSuite.cpp

  # API Tests
  api/berryIPageLayoutTest.cpp
  api/berryIViewPartTest.cpp
  api/berryIWorkbenchPartTest.cpp
  api/berryMockSelectionProvider.cpp
  api/berryMockViewPart.cpp
  api/berryMockWorkbenchPart.cpp
  api/berrySaveableMockViewPart.cpp
  api/berryUiApiTestSuite.cpp
  api/berryXMLMementoTest.cpp
  
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

