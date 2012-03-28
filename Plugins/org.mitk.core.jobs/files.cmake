set(SRC_CPP_FILES

)
set(INTERNAL_CPP_FILES
  mitkDataStorageAccessRule.cpp
  mitkPluginActivator.cpp
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
