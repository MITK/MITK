# pybind11 extension modules resolve Python C API symbols at runtime from
# the hosting Python interpreter, not from a linked libpython. Strip MITK's
# global -Wl,--no-undefined from CMAKE_MODULE_LINKER_FLAGS at directory scope
# so the linker allows unresolved Python C API symbols in this target.
string(REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")

pybind11_add_module(mitk_python_bindings
  CppMicroServices.cpp
  Exception.cpp
  Geometries.cpp
  Image.cpp
  IOUtil.cpp
  Module.cpp
  PixelType.cpp
  PixelType.h
  Points.cpp
  Property.cpp
  PropertyList.cpp
  PropertySerialization.cpp
  PropertyAutoWrap.h
  PropertyConversionUtils.h
  PropertyOwnerBindings.h
  SmartPointer.h
  Vectors.cpp
)

target_link_libraries(mitk_python_bindings
  PRIVATE MitkCore Python3::Module
)

set_target_properties(mitk_python_bindings PROPERTIES
  FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
  OUTPUT_NAME "mitk"
)

set(mitk_PACKAGE_DIR "${Python3_SITEARCH}/mitk")
file(MAKE_DIRECTORY "${mitk_PACKAGE_DIR}")

file(WRITE "${mitk_PACKAGE_DIR}/__init__.py" "from .mitk import *\n")

# Copy the property_view.py file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/property_view.py"
  "${mitk_PACKAGE_DIR}/property_view.py"
  COPYONLY
)

# Copy the test file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_properties.py"
  "${mitk_PACKAGE_DIR}/tests/test_properties.py"
  COPYONLY
)

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

if(APPLE)
  set_target_properties(mitk_python_bindings PROPERTIES
    INSTALL_RPATH "@loader_path"
  )
elseif(NOT WIN32)
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
  "${CMAKE_CURRENT_BINARY_DIR}/wheel_init/__init__.py"
  @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wheel_init/__init__.py"
  DESTINATION mitk
  COMPONENT wheel
  EXCLUDE_FROM_ALL
)

# Install auto-load modules into their expected subdirectories.
# CppMicroServices looks for <module_name>/ subdirectory relative to
# US_AUTOLOAD_PATHS (set to the package directory in __init__.py).

get_property(_mitk_module_targets GLOBAL PROPERTY MITK_MODULE_TARGETS)

set(_wheel_autoload_targets "")

foreach(_target IN LISTS _mitk_module_targets)
  get_target_property(_autoload_dir ${_target} MITK_AUTOLOAD_DIRECTORY)

  if(_autoload_dir AND NOT "${_target}" STREQUAL "MitkPreloadPython")
    install(TARGETS ${_target}
      COMPONENT wheel
      EXCLUDE_FROM_ALL
      RUNTIME DESTINATION mitk/${_autoload_dir}
      LIBRARY DESTINATION mitk/${_autoload_dir}
    )
    list(APPEND _wheel_autoload_targets ${_target})
  endif()
endforeach()

# ---------------------------------------------------------------------------
# Wheel build target (PythonWheel configuration only)
# ---------------------------------------------------------------------------

if(MITK_BUILD_CONFIGURATION STREQUAL "PythonWheel")
  set(_wheel_script "${CMAKE_CURRENT_SOURCE_DIR}/../wheel/build_wheel.py")

  if(WIN32)
    set(_wheel_pip_deps wheel delvewheel)
  elseif(APPLE)
    set(_wheel_pip_deps wheel delocate)
  else()
    set(_wheel_pip_deps wheel auditwheel patchelf)
  endif()

  add_custom_target(mitk_python_wheel ALL
    COMMAND ${Python3_EXECUTABLE} -m pip install --quiet ${_wheel_pip_deps}
    COMMAND ${Python3_EXECUTABLE} ${_wheel_script} --build-dir ${CMAKE_BINARY_DIR} --cmake ${CMAKE_COMMAND}
    COMMENT "Building MITK Python wheel"
    DEPENDS mitk_python_bindings ${_wheel_autoload_targets}
  )

  set_target_properties(mitk_python_wheel PROPERTIES
    FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
  )
endif()

# ---------------------------------------------------------------------------
# Virtual environment setup
# ---------------------------------------------------------------------------
# Resolve all runtime paths at configure time, then bake them into a
# self-contained cmake script that cmake -P can execute at build time without
# access to the cache or any find_package results.
# ---------------------------------------------------------------------------

mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH "release" RELEASE)

# Format DLL paths as Python list entries for sitecustomize.py.
set(_mitk_dll_paths_python_list "")
foreach(_path IN LISTS MITK_RUNTIME_PATH)
  string(APPEND _mitk_dll_paths_python_list "    r'${_path}',\n")
endforeach()

# The site-packages subdirectory layout differs between Windows and POSIX venvs.
if(WIN32)
  set(_venv_sitecustomize_subdir "Lib/site-packages")
else()
  set(_venv_sitecustomize_subdir "lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")
endif()

# Bake all configure-time values into a self-contained build-time script.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup_venv.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/setup_venv_resolved.cmake"
  @ONLY
)

add_custom_target(mitk_python_venv
  COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/setup_venv_resolved.cmake"
  COMMENT "Setting up virtual environment for MITK Python bindings"
  DEPENDS mitk_python_bindings
)

set_target_properties(mitk_python_venv PROPERTIES
  FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
)
