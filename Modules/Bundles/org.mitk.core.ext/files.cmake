SET(SRC_H_FILES
  mitkIInputDeviceDescriptor.h
  mitkIInputDeviceRegistry.h
  mitkIInputDevice.h
)

SET(SRC_CPP_FILES
  mitkCoreExtConstants.cpp
  mitkInputDeviceRegistry.cpp
  mitkInputDeviceDescriptor.cpp
)

SET(INTERNAL_CPP_FILES
  mitkCoreExtActivator.cpp
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(MOC_H_FILES
  src/internal/mitkCoreExtActivator.h
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

