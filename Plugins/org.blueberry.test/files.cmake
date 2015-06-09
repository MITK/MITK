set(MOC_H_FILES
  src/internal/berryPluginActivator.h

  src/berryCoreTestApplication.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryCoreTestApplication.cpp
  berryBlueBerryTestDriver.cpp

  harness/berryTestCase.cpp

  util/berryCallHistory.cpp
)

set(INTERNAL_CPP_FILES
  berryPluginActivator.cpp
  berryTestDescriptor.cpp
  berryTestRegistry.cpp
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

