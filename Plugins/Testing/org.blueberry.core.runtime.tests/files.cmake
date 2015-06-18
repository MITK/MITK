set(MOC_H_FILES
  src/berryCoreRuntimeTestSuite.h
  src/berryPluginActivator.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryCoreRuntimeTestSuite.cpp
  berryPreferencesServiceTest.cpp
  berryPreferencesTest.cpp

  berryPluginActivator.cpp
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
