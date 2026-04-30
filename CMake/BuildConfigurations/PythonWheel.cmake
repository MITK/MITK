# Build configuration for creating a standalone Python wheel.
# Headless: no Qt, no BlueBerry, no plugins. Release only.
# Usage: cmake -DMITK_BUILD_CONFIGURATION=PythonWheel ...

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release")
endif()

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

set(MITK_USE_BLUEBERRY OFF CACHE BOOL "" FORCE)
set(MITK_USE_CTK OFF CACHE BOOL "" FORCE)
set(MITK_USE_Qt6 OFF CACHE BOOL "" FORCE)
set(MITK_USE_Qt6Qwt6 OFF CACHE BOOL "" FORCE)

set(MITK_CONFIG_PACKAGES
  Python3
  pybind11
)
