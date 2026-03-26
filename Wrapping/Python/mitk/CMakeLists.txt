pybind11_add_module(mitk_python_bindings
  CppMicroServices.cpp
  Exception.cpp
  Geometries.cpp
  Image.cpp
  Module.cpp
  PixelType.cpp
  PixelType.h
  Points.cpp
  SmartPointer.h
  Vectors.cpp
)

target_link_libraries(mitk_python_bindings
  PRIVATE MitkCore Python3::Python
)

set_target_properties(mitk_python_bindings PROPERTIES
  FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
  OUTPUT_NAME "mitk"
)

set(mitk_PACKAGE_DIR "${Python3_SITEARCH}/mitk")
file(MAKE_DIRECTORY "${mitk_PACKAGE_DIR}")

file(WRITE "${mitk_PACKAGE_DIR}/__init__.py" "from .mitk import *\n")

if(MSVC)
  set_target_properties(mitk_python_bindings PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${mitk_PACKAGE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${mitk_PACKAGE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${mitk_PACKAGE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${mitk_PACKAGE_DIR}"
  )
else()
  set_target_properties(mitk_python_bindings PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${mitk_PACKAGE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${mitk_PACKAGE_DIR}"
  )
endif()

# ---------------------------------------------------------------------------
# Wheel packaging support
# ---------------------------------------------------------------------------
# The 'wheel' install component stages the Python bindings and auto-load
# modules for packaging into a redistributable wheel. It is EXCLUDE_FROM_ALL
# so it does not interfere with the default install.
#
# Usage:
#   cmake --install <build-dir> --component wheel --prefix <staging-dir>
#   python Wrapping/Python/wheel/build_wheel.py --build-dir <build-dir>
# ---------------------------------------------------------------------------

if(NOT WIN32)
  set_target_properties(mitk_python_bindings PROPERTIES
    INSTALL_RPATH "$ORIGIN"
  )
endif()

install(TARGETS mitk_python_bindings
  COMPONENT wheel
  EXCLUDE_FROM_ALL
  RUNTIME DESTINATION mitk
  LIBRARY DESTINATION mitk
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/../wheel/__init__.py.in"
  "${CMAKE_CURRENT_BINARY_DIR}/wheel/__init__.py"
  @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wheel/__init__.py"
  DESTINATION mitk
  COMPONENT wheel
  EXCLUDE_FROM_ALL
)

# Install auto-load modules into their expected subdirectories.
# CppMicroServices looks for <module_name>/ subdirectory relative to
# US_AUTOLOAD_PATHS (set to the package directory in __init__.py).

get_property(_mitk_module_targets GLOBAL PROPERTY MITK_MODULE_TARGETS)

foreach(_target IN LISTS _mitk_module_targets)
  get_target_property(_autoload_dir ${_target} MITK_AUTOLOAD_DIRECTORY)

  if(_autoload_dir AND NOT "${_target}" STREQUAL "MitkPreloadPython")
    install(TARGETS ${_target}
      COMPONENT wheel
      EXCLUDE_FROM_ALL
      RUNTIME DESTINATION mitk/${_autoload_dir}
      LIBRARY DESTINATION mitk/${_autoload_dir}
    )
  endif()
endforeach()
