# pybind11 extension modules resolve Python C API symbols at runtime from
# the hosting Python interpreter, not from a linked libpython. Strip MITK's
# global -Wl,--no-undefined from CMAKE_MODULE_LINKER_FLAGS at directory scope
# so the linker allows unresolved Python C API symbols in this target.
string(REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")

set(mitk_binding_sources
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

pybind11_add_module(mitk_python_bindings ${mitk_binding_sources})

target_link_libraries(mitk_python_bindings
  PRIVATE MitkCore MitkDICOM MitkMultilabel Python3::Module
)

set_target_properties(mitk_python_bindings PROPERTIES
  FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
  OUTPUT_NAME "mitk"
)

set(mitk_PACKAGE_DIR "${Python3_SITEARCH}/mitk")
file(MAKE_DIRECTORY "${mitk_PACKAGE_DIR}")

# Build-tree __init__.py for the mitk package, configured from the shared
# `__init__.py.in` template used for both the build tree and the wheel.
# Re-exports the names from the compiled pybind11 extension (the inner
# `mitk` module inside the `mitk` package), rewrites their ``__module__``
# attribute from ``"mitk.mitk"`` to ``"mitk"`` so that Sphinx, IDE tooling,
# and ``repr()`` render ``mitk.Image`` rather than ``mitk.mitk.Image``, and
# exposes ``__version__``. The wheel-detection branch in the template is a
# no-op in the build tree because none of the .libs / .dylibs directories
# the platform delocators create are present here.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${mitk_PACKAGE_DIR}/__init__.py"
  @ONLY
)

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

set_target_properties(mitk_mxn_layout_staged PROPERTIES
  FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
)

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
# Additional CPython ABIs
#
# pybind11 has no stable-ABI support, so a wheel is only installable on the
# CPython series it was compiled against. The extension module is the single
# Python-version-dependent target in the whole build, so one build tree can
# serve several series by compiling it once per series; everything else the
# wheel contains is version agnostic and built once.
#
# The additional interpreters are staged by the superbuild
# (CMakeExternals/Python3.cmake) into MITK-build/python-<version>.
# ---------------------------------------------------------------------------

# Wheel ABI tag of a CPython version, e.g. 3.13 -> cp313.
function(mitk_python_abi_tag version output_var)
  string(REPLACE "." "" _digits "${version}")
  set(${output_var} "cp${_digits}" PARENT_SCOPE)
endfunction()

#[[ Builds another copy of the extension module against a different CPython
    series.

    pybind11_add_module() cannot be reused: it delegates to
    python3_add_library(), which links the one Python3::Module target that
    find_package(Python3) created for the embedded interpreter. Everything else
    it contributes is version agnostic and replicated here.

    Headers are enough to compile against, because the extension resolves Python
    C API symbols from the hosting interpreter; only MSVC additionally needs an
    import library. Include directory and module suffix are read from the target
    interpreter rather than derived from the version, so the resulting file name
    is whatever that interpreter's import machinery actually looks for. ]]
function(mitk_add_python_abi_extension version)
  mitk_python_abi_tag(${version} abi)

  set(root "${MITK_BINARY_DIR}/python-${version}")

  if(WIN32)
    set(interpreter "${root}/python.exe")
  else()
    set(interpreter "${root}/bin/python3")
  endif()

  if(NOT EXISTS "${interpreter}")
    message(FATAL_ERROR "No CPython ${version} interpreter at ${interpreter}. Interpreters for "
      "additional ABIs are staged by the superbuild from MITK_Python3_WHEEL_VERSIONS.")
  endif()

  execute_process(
    COMMAND "${interpreter}" -c
      "import sysconfig as s; print(s.get_paths()['include']); print(s.get_config_var('EXT_SUFFIX'))"
    OUTPUT_VARIABLE probe
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE probe_result
  )

  if(NOT probe_result EQUAL 0)
    message(FATAL_ERROR "Failed to query sysconfig from ${interpreter}")
  endif()

  string(REGEX REPLACE "\r?\n" ";" probe "${probe}")
  list(GET probe 0 include_dir)
  list(GET probe 1 ext_suffix)
  file(TO_CMAKE_PATH "${include_dir}" include_dir)

  set(target mitk_python_bindings_${abi})

  add_library(${target} MODULE ${mitk_binding_sources})

  target_include_directories(${target} SYSTEM PRIVATE "${include_dir}")

  target_link_libraries(${target} PRIVATE
    MitkCore
    MitkDICOM
    MitkMultilabel
    pybind11::headers
    pybind11::python_link_helper
  )

  if(MSVC)
    string(REPLACE "." "" version_digits "${version}")
    target_link_libraries(${target} PRIVATE
      "${root}/libs/python${version_digits}.lib"
      pybind11::windows_extras
    )
  endif()

  if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    target_link_libraries(${target} PRIVATE pybind11::lto)
  endif()

  # Kept out of the embedded interpreter's site-packages: only a matching
  # interpreter can import these, and the wheel collects them by component.
  #
  # Every ABI shares the module name, so the import library MSVC generates
  # beside the module needs its own directory too: with the default archive
  # directory all of them would write lib/<config>/mitk.lib and race.
  set(output_dir "${CMAKE_BINARY_DIR}/Wrapping/wheel/${abi}/mitk")
  set(archive_dir "${CMAKE_BINARY_DIR}/Wrapping/wheel/${abi}")

  set_target_properties(${target} PROPERTIES
    PREFIX ""
    SUFFIX "${ext_suffix}"
    OUTPUT_NAME "mitk"
    FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
    RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
    LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
    ARCHIVE_OUTPUT_DIRECTORY "${archive_dir}"
  )

  if(MSVC)
    foreach(config RELEASE RELWITHDEBINFO)
      set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config} "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY_${config} "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY_${config} "${archive_dir}"
      )
    endforeach()
  endif()

  if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET)
    set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET "hidden")
  endif()

  if(APPLE)
    set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path")
  elseif(NOT WIN32)
    set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
  endif()

  if(DEFINED CMAKE_BUILD_TYPE AND NOT MSVC)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
    if(NOT build_type MATCHES "DEBUG|RELWITHDEBINFO|NONE")
      pybind11_strip(${target})
    endif()
  endif()
endfunction()

# ---------------------------------------------------------------------------
# Wheel packaging support (PythonWheel configuration only)
# ---------------------------------------------------------------------------
# The install components stage the Python bindings and auto-load modules for
# packaging into a redistributable wheel. Everything shared by all wheels goes
# into 'wheel'; each extension module goes into its own 'wheel_<abi>', so
# build_wheel.py stages 'wheel' plus the one ABI it is packing.
#
# Guarded by MITK_BUILD_CONFIGURATION to prevent the wheel components from
# leaking into regular builds. CPack's DragNDrop generator on macOS switches
# into component-aware mode when it detects any named component, creating an
# ALL_IN_ONE folder that breaks the app bundle layout.
#
# Usage:
#   cmake --install <build-dir> --component wheel --prefix <staging-dir>
#   cmake --install <build-dir> --component wheel_cp312 --prefix <staging-dir>
#   python Wrapping/Python/wheel/build_wheel.py --build-dir <build-dir>
#
# Set MITK_WHEEL_VERSION (or pass build_wheel.py --version) to pin an
# explicit PEP 440 wheel version; empty derives it from git.
# ---------------------------------------------------------------------------

if(MITK_BUILD_CONFIGURATION STREQUAL "PythonWheel")
  # Declared so the value (forwarded from the superbuild, or set directly on
  # this build) is stored as MITK_WHEEL_VERSION:STRING in CMakeCache.txt for
  # build_wheel.py to read. Empty derives the version from git.
  set(MITK_WHEEL_VERSION "" CACHE STRING
    "Override the generated Python wheel version (PEP 440 public version); empty derives from git")

  # The superbuild owns this list and forwards its value; the default here only
  # applies to an inner build configured by hand, which has no staged
  # interpreters for any other series.
  set(MITK_Python3_WHEEL_VERSIONS "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}"
    CACHE STRING "CPython versions to build mitk Python wheels for")
  mark_as_advanced(MITK_Python3_WHEEL_VERSIONS)

  mitk_python_abi_tag("${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}" mitk_primary_abi)

  set(mitk_extra_wheel_versions ${MITK_Python3_WHEEL_VERSIONS})
  list(REMOVE_ITEM mitk_extra_wheel_versions
    "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

  foreach(_version IN LISTS mitk_extra_wheel_versions)
    mitk_add_python_abi_extension(${_version})
  endforeach()

  if(APPLE)
    set_target_properties(mitk_python_bindings PROPERTIES
      INSTALL_RPATH "@loader_path"
    )
  elseif(NOT WIN32)
    set_target_properties(mitk_python_bindings PROPERTIES
      INSTALL_RPATH "$ORIGIN"
    )
  endif()

  # One component per ABI, so staging a wheel picks up exactly one extension.
  install(TARGETS mitk_python_bindings
    COMPONENT wheel_${mitk_primary_abi}
    EXCLUDE_FROM_ALL
    RUNTIME DESTINATION mitk
    LIBRARY DESTINATION mitk
  )

  foreach(_version IN LISTS mitk_extra_wheel_versions)
    mitk_python_abi_tag(${_version} _abi)
    install(TARGETS mitk_python_bindings_${_abi}
      COMPONENT wheel_${_abi}
      EXCLUDE_FROM_ALL
      RUNTIME DESTINATION mitk
      LIBRARY DESTINATION mitk
    )
  endforeach()

  # Reuse the same template the build-tree __init__.py is configured from;
  # the wheel-detection branch in the template activates at runtime when the
  # bundled .libs / .dylibs directory is present alongside the package.
  install(FILES "${mitk_PACKAGE_DIR}/__init__.py"
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

  # Wheel install for the mxn DSL subpackage. `examples/` is excluded --
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
  #
  # One build_wheel.py run per requested ABI, sequentially. The packaging
  # dependencies are installed once into the embedded interpreter, which drives
  # every run: the delocators inspect binaries and do not care which
  # interpreter hosts them, and only that interpreter is provisioned with pip
  # packages.

  set(_wheel_script "${CMAKE_CURRENT_SOURCE_DIR}/../wheel/build_wheel.py")

  if(WIN32)
    set(_wheel_pip_deps wheel delvewheel)
  elseif(APPLE)
    set(_wheel_pip_deps wheel delocate)
  else()
    set(_wheel_pip_deps wheel auditwheel patchelf)
  endif()

  set(_wheel_commands COMMAND ${Python3_EXECUTABLE} -m pip install --quiet ${_wheel_pip_deps})
  set(_wheel_abi_targets mitk_python_bindings)

  foreach(_version IN LISTS MITK_Python3_WHEEL_VERSIONS)
    mitk_python_abi_tag(${_version} _abi)
    list(APPEND _wheel_commands
      COMMAND ${Python3_EXECUTABLE} ${_wheel_script}
              --build-dir ${CMAKE_BINARY_DIR}
              --cmake ${CMAKE_COMMAND}
              --abi ${_abi}
    )
    if(NOT _abi STREQUAL mitk_primary_abi)
      list(APPEND _wheel_abi_targets mitk_python_bindings_${_abi})
    endif()
  endforeach()

  add_custom_target(mitk_python_wheel ALL
    ${_wheel_commands}
    COMMENT "Building MITK Python wheels"
    DEPENDS ${_wheel_abi_targets} ${_wheel_autoload_targets}
  )

  set_target_properties(mitk_python_wheel PROPERTIES
    FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
  )
endif()
