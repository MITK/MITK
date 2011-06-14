SET(MOC_H_FILES
  src/internal/berryPluginActivator.h
  
  src/berryCoreTestApplication.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(SRC_CPP_FILES
  berryCoreTestApplication.cpp
  berryBlueBerryTestDriver.cpp
  
  harness/berryTestCase.cpp
  
  util/berryCallHistory.cpp
)

SET(INTERNAL_CPP_FILES
  berryPluginActivator.cpp
  berryTestDescriptor.cpp
  berryTestRegistry.cpp
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

