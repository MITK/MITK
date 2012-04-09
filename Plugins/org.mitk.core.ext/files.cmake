set(SRC_H_FILES
  mitkIInputDeviceDescriptor.h
  mitkIInputDeviceRegistry.h
  mitkIInputDevice.h
)

set(SRC_CPP_FILES
  mitkCoreExtConstants.cpp
  mitkInputDeviceRegistry.cpp
  mitkInputDeviceDescriptor.cpp
)

set(INTERNAL_CPP_FILES
  mitkCoreExtActivator.cpp
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(MOC_H_FILES
  src/internal/mitkCoreExtActivator.h
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

