set(MOC_H_FILES
  src/internal/berryPluginActivator.h
)

set(SRC_CPP_FILES
  berryMultiStatus.cpp
  berrySafeRunner.cpp
  berrySolsticeExceptions.cpp
  berryStatus.cpp
)

set(INTERNAL_CPP_FILES
  berryIRuntimeConstants.cpp
  berryPluginActivator.cpp
)

#set(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
