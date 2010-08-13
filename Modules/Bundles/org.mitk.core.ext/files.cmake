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

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

