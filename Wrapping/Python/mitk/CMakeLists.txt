# pybind11 extension modules resolve Python C API symbols at runtime from
# the hosting Python interpreter, not from a linked libpython. Strip MITK's
# global -Wl,--no-undefined from CMAKE_MODULE_LINKER_FLAGS at directory scope
# so the linker allows unresolved Python C API symbols in this target.
string(REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")

pybind11_add_module(mitk_python_bindings
  Color.cpp
  CppMicroServices.cpp
  Exception.cpp
  Geometries.cpp
  GeometryHelpers.cpp
  GeometryHelpers.h
  Image.cpp
  IOUtil.cpp
  Module.cpp
  MultiLabelSegmentation.cpp
  PixelType.cpp
  PixelType.h
  Points.cpp
  Property.cpp
  PropertyList.cpp
  PropertyAutoWrap.h
  PropertyNotOwnedError.h
  PropertyOwnerBindings.h
  SmartPointer.h
  Vectors.cpp
  TemporoSpatialStringProperty.cpp
  PropertyKeyPath.cpp
  DICOMTagPath.cpp
  Dicom.cpp
  Relations.cpp
  TemporoSpatialStringSerialization.h
)

target_link_libraries(mitk_python_bindings
  PRIVATE MitkCore MitkDICOM MitkMultilabel Python3::Module
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

# ---------------------------------------------------------------------------
# Stage the pure-Python `mitk.mxn.layout` DSL package.
#
# Per-file `add_custom_command` so that editing a `.py` source file
# triggers a copy_if_different on the next `cmake --build`, without a
# CMake re-configure pass. Adding or removing a `.py` file from
# _mitk_layout_py still requires re-configure (CMake needs to learn the
# new dependency).
# ---------------------------------------------------------------------------

set(_mitk_layout_py
  __init__.py
  _vocabulary.py
  _model.py
  _builders.py
  _presets.py
  _selector.py
  _io.py
  _repr.py
  _validation.py
)

file(MAKE_DIRECTORY "${mitk_PACKAGE_DIR}/mxn/layout")

set(_mitk_layout_staged_files)
foreach(_py IN LISTS _mitk_layout_py)
  set(_src "${CMAKE_CURRENT_SOURCE_DIR}/mxn/layout/${_py}")
  set(_dst "${mitk_PACKAGE_DIR}/mxn/layout/${_py}")
  add_custom_command(
    OUTPUT  "${_dst}"
    DEPENDS "${_src}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
    COMMENT "Staging mitk.mxn.layout/${_py}"
    VERBATIM
  )
  list(APPEND _mitk_layout_staged_files "${_dst}")
endforeach()

# `mitk/mxn/__init__.py` is the namespace marker for the parent package.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/mxn/__init__.py"
  "${mitk_PACKAGE_DIR}/mxn/__init__.py"
  COPYONLY
)

add_custom_target(mitk_mxn_layout_staged
                  DEPENDS ${_mitk_layout_staged_files})
add_dependencies(mitk_python_bindings mitk_mxn_layout_staged)

# The DSL examples are smoke-tested as part of the `mitkPythonBindingsTest`
# pytest suite (see `Modules/Python/test/pytest/test_layout/
# test_examples_smoke.py`). That suite runs inside the C++ test driver, which
# embeds Python in a process that has already loaded MITK's runtime DLLs --
# the only environment where `import mitk` is reliably resolvable on Windows.


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

# The MitkPython test driver hosts mitkPythonBindingsTest, which imports the
# 'mitk' Python package at runtime. Ensure the bindings are built before the
# test driver. Declared here (not in the test CMakeLists) because the Modules
# subtree is configured before Wrapping, so mitk_python_bindings does not yet
# exist at test-registration time.
if(TARGET MitkPythonTestDriver)
  add_dependencies(MitkPythonTestDriver mitk_python_bindings)
endif()

# ---------------------------------------------------------------------------
# Wheel packaging support (PythonWheel configuration only)
# ---------------------------------------------------------------------------
# The 'wheel' install component stages the Python bindings and auto-load
# modules for packaging into a redistributable wheel.
#
# Guarded by MITK_BUILD_CONFIGURATION to prevent the 'wheel' component from
# leaking into regular builds. CPack's DragNDrop generator on macOS switches
# into component-aware mode when it detects any named component, creating an
# ALL_IN_ONE folder that breaks the app bundle layout.
#
# Usage:
#   cmake --install <build-dir> --component wheel --prefix <staging-dir>
#   python Wrapping/Python/wheel/build_wheel.py --build-dir <build-dir>
# ---------------------------------------------------------------------------

if(MITK_BUILD_CONFIGURATION STREQUAL "PythonWheel")
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

install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/property_view.py"
  DESTINATION mitk
  COMPONENT wheel
  EXCLUDE_FROM_ALL
)

# Wheel install for the mxn DSL subpackage. `examples/` is excluded —
# example files are pedagogy, not part of the redistributable surface.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/mxn/"
  DESTINATION mitk/mxn
  COMPONENT wheel
  EXCLUDE_FROM_ALL
  FILES_MATCHING
    PATTERN "*.py"
    PATTERN "examples" EXCLUDE
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

  # Wheel build target

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
