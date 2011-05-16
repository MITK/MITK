SET(MOC_H_FILES
  src/berryOSGiCoreTestSuite.h
  src/berryPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(SRC_CPP_FILES
  berryMessageTest.cpp
  berryObjectTest.cpp
  berryOSGiCoreTestSuite.cpp
  berryPluginActivator.cpp
  berrySmartPointerTest.cpp
  berryWeakPointerTest.cpp
)

SET(INTERNAL_CPP_FILES
  
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
