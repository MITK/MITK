set(MOC_H_FILES
  src/internal/berryPluginActivator.h
  src/util/berryEmptyPerspective.h

  src/berryUITestApplication.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryUITestApplication.cpp

  harness/berryUITestCase.cpp

  util/berryEmptyPerspective.cpp
)

set(INTERNAL_CPP_FILES
  berryPluginActivator.cpp
  berryUITestWorkbenchAdvisor.cpp
  berryUITestWorkbenchWindowAdvisor.cpp
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

