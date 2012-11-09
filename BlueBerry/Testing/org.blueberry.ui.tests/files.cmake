set(MOC_H_FILES
  src/berryPluginActivator.h
  src/berryUiTestSuite.h

  src/api/berryMockViewPart.h
  src/api/berrySaveableMockViewPart.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  icons/view.gif
)

set(SRC_CPP_FILES

  berryPluginActivator.cpp
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

set(INTERNAL_CPP_FILES

)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

