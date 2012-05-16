set(MOC_H_FILES
  src/internal/berryPluginActivator.h
  src/internal/berryPreferencesService.h
)

set(SRC_CPP_FILES
  berryIAdaptable.cpp
  berryIAdapterManager.cpp
  berryIPreferences.cpp
  berryIBerryPreferences.cpp
  berryIPreferencesService.cpp
  berryPlatformObject.cpp
  berryRuntime.cpp
  berryBackingStoreException.cpp
)

set(INTERNAL_CPP_FILES
  berryAbstractPreferencesStorage.cpp
  berryPluginActivator.cpp
  berryPreferences.cpp
  berryPreferencesService.cpp
  berryXMLPreferencesStorage.cpp
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
