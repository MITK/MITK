# Build configuration for creating a standalone Python wheel.
# Headless: no Qt, no BlueBerry, no plugins.
# Usage: cmake -DMITK_BUILD_CONFIGURATION=PythonWheel ...

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

set(MITK_USE_BLUEBERRY OFF CACHE BOOL "" FORCE)
set(MITK_USE_CTK OFF CACHE BOOL "" FORCE)
set(MITK_USE_Qt6 OFF CACHE BOOL "" FORCE)
set(MITK_USE_Qt6Qwt6 OFF CACHE BOOL "" FORCE)

set(MITK_CONFIG_PACKAGES
  Python3
  pybind11
)
