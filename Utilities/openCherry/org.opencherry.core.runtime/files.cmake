SET(CPP_FILES
  manifest.cpp
  cherryIAdapterManager.cpp
  cherryRuntime.cpp
  cherryRuntimePlugin.cpp
)

SET(INTERNAL_CPP_FILES
  
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

