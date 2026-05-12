find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
add_subdirectory(mitk)

# ---------------------------------------------------------------------------
# Sphinx documentation for the mitk Python package
# ---------------------------------------------------------------------------
# The mitk_python_docs target invokes sphinx-build to produce the HTML
# documentation for the mitk Python wheel. Off-by-default; invoke with:
#
#   cmake --build <build-dir> --target mitk_python_docs
#
# The target maintains a dedicated venv at <build>/mitk_python_docs_venv
# based on the standalone MITK Python. --system-site-packages lets the venv
# see the freshly-built mitk package without an extra install step; -s on
# the python invocations keeps the user site-packages dir out of sys.path
# so the Sphinx toolchain is sourced solely from this venv (the standalone
# Python's user-site overlaps with the system Python's on Windows, which
# is exactly the kind of leak we want to avoid for reproducible doc builds).
#
# Autodoc imports the mitk package at build time to extract docstrings from
# the compiled extension. The dependency on mitk_python_bindings ensures the
# extension and its in-tree __init__.py are staged into the build Python's
# site-packages before the docs build runs. On Windows, Python 3.8+ ignores
# PATH for native-extension DLL resolution, so the docs/_run_sphinx.py
# launcher calls os.add_dll_directory() on each path returned by
# mitkFunctionGetLibrarySearchPaths -- the same helper MITK_CREATE_MODULE_TESTS
# uses for module tests. Linux and macOS rely on the BUILD_RPATH baked into
# the extension by CMake.
# ---------------------------------------------------------------------------

set(_python_docs_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/docs")
set(_python_docs_output_dir "${CMAKE_BINARY_DIR}/Documentation/Python/html")
set(_python_docs_requirements "${_python_docs_source_dir}/requirements.txt")
set(_python_docs_venv "${CMAKE_BINARY_DIR}/mitk_python_docs_venv")

if(WIN32)
  set(_python_docs_venv_python "${_python_docs_venv}/Scripts/python.exe")
else()
  set(_python_docs_venv_python "${_python_docs_venv}/bin/python")
endif()

# Reuse the same library-search-path helper that MITK_CREATE_MODULE_TESTS
# uses to populate PATH for module tests. The list is the same one that
# lets mitkPythonBindingsTest find MitkCore / MitkDICOM / ITK / VTK /
# Python / Qt / CTK / BlueBerry / external-project DLLs on Windows.
#
# Python 3.8+ on Windows ignores PATH when resolving DLL dependencies of
# native extension modules, so we pass the paths to _run_sphinx.py as
# --dll-dir arguments and let it call os.add_dll_directory before
# Sphinx's autodoc imports the mitk extension.
include(mitkFunctionGetLibrarySearchPaths)
mitkFunctionGetLibrarySearchPaths(_python_docs_runtime_paths_release release RELEASE)

set(_python_docs_dll_dir_args)
if(WIN32)
  foreach(_path IN LISTS _python_docs_runtime_paths_release)
    list(APPEND _python_docs_dll_dir_args --dll-dir "${_path}")
  endforeach()
endif()
# On Linux and macOS, the compiled extension's BUILD_RPATH (set by CMake
# when linking against MITK targets) already resolves the dependent
# libraries from the build tree, so no extra discovery is needed.

add_custom_target(mitk_python_docs
  COMMAND ${Python3_EXECUTABLE} -m venv --system-site-packages ${_python_docs_venv}
  COMMAND ${_python_docs_venv_python} -s -m pip install --quiet
          --disable-pip-version-check
          -r ${_python_docs_requirements}
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${_python_docs_source_dir}/api/generated"
  COMMAND ${_python_docs_venv_python} -s
          "${_python_docs_source_dir}/_run_sphinx.py"
          ${_python_docs_dll_dir_args}
          --no-color --quiet
          -W --keep-going
          -b html
          ${_python_docs_source_dir}
          ${_python_docs_output_dir}
  WORKING_DIRECTORY ${_python_docs_source_dir}
  COMMENT "Building MITK Python documentation (Sphinx)"
  DEPENDS mitk_python_bindings
)

set_target_properties(mitk_python_docs PROPERTIES
  FOLDER "${MITK_ROOT_FOLDER}/Wrapping"
)
