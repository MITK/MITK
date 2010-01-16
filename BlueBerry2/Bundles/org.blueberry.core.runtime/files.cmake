SET(SRC_CPP_FILES
  berryIAdaptable.cpp
  berryIAdapterManager.cpp
  berryIPreferencesService.cpp
  berryPlatformObject.cpp
  berryRuntime.cpp
  berryRuntimePlugin.cpp
  berryBackingStoreException.cpp
)

SET(INTERNAL_CPP_FILES
  berryPreferencesService.cpp
  berryAbstractPreferencesStorage.cpp  
  berryPreferences.cpp
  berryXMLPreferencesStorage.cpp
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
