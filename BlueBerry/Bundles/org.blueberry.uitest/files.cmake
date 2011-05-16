SET(MOC_H_FILES
  src/internal/berryPluginActivator.h
  src/util/berryEmptyPerspective.h
  
  src/berryUITestApplication.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(SRC_CPP_FILES
  berryUITestApplication.cpp
  
  harness/berryUITestCase.cpp
  
  util/berryEmptyPerspective.cpp
)

SET(INTERNAL_CPP_FILES
  berryPluginActivator.cpp
  berryUITestWorkbenchAdvisor.cpp
  berryUITestWorkbenchWindowAdvisor.cpp
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

