
set(SRC_CPP_FILES
  mitkCoreExtConstants.cpp
  mitkIInputDeviceDescriptor.h
  mitkIInputDeviceRegistry.h
  mitkIInputDevice.h
)

set(INTERNAL_CPP_FILES
  mitkCoreExtActivator.cpp
  mitkInputDeviceRegistry.cpp
  mitkInputDeviceDescriptor.cpp
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(MOC_H_FILES
  src/internal/mitkCoreExtActivator.h
  src/internal/mitkInputDeviceRegistry.h
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

