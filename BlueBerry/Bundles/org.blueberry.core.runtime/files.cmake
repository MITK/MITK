SET(MOC_H_FILES
  src/internal/berryPluginActivator.h
  src/internal/berryPreferencesService.h
)

SET(SRC_CPP_FILES
  berryIAdaptable.cpp
  berryIAdapterManager.cpp
  berryIPreferencesService.cpp
  berryPlatformObject.cpp
  berryRuntime.cpp
  berryBackingStoreException.cpp
)

SET(INTERNAL_CPP_FILES
  berryAbstractPreferencesStorage.cpp
  berryPluginActivator.cpp
  berryPreferences.cpp
  berryPreferencesService.cpp
  berryXMLPreferencesStorage.cpp
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
