if(APPLE)
  message(STATUS "Module GPGPU is not ported to MacOS yet")
else(APPLE)

set(package_deps)
if(UNIX)
  list(APPEND package_deps X11)
endif()

MITK_CREATE_MODULE(mitkGPGPU
#  INCLUDE_DIRS .
  DEPENDS Mitk glew
  PACKAGE_DEPENDS QT ${package_deps}
  EXPORT_DEFINE GPGPU_DLL_API
  QT_MODULE
)

endif(APPLE)

