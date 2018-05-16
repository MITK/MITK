if(APPLE)
  message(STATUS "Module GPGPU is not ported to MacOS yet")
else(APPLE)

set(package_deps)
if(UNIX)
  list(APPEND package_deps X11)
endif()

MITK_CREATE_MODULE(
#  INCLUDE_DIRS .
  DEPENDS MitkCore
  PACKAGE_DEPENDS ${package_deps} OpenGL Qt5|Widgets
)

endif(APPLE)

