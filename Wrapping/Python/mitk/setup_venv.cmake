# CMake script template — processed by configure_file(@ONLY) in CMakeLists.txt.
# All @VAR@ placeholders are substituted with build-tree values at configure time.
# The resolved copy is written to the binary dir and invoked via cmake -P at build time.
# Edit this source file, not the generated copy in the build tree.

set(_venv_dir "@CMAKE_BINARY_DIR@/python_venv")

if(EXISTS "${_venv_dir}")
  message(STATUS "Removing old virtual environment at ${_venv_dir}")
  file(REMOVE_RECURSE "${_venv_dir}")
endif()

message(STATUS "Creating virtual environment at ${_venv_dir}")
execute_process(
  COMMAND "@Python3_EXECUTABLE@" -m venv "${_venv_dir}"
  RESULT_VARIABLE _venv_result
)
if(NOT _venv_result EQUAL 0)
  message(FATAL_ERROR "Failed to create virtual environment at ${_venv_dir}")
endif()

# Write sitecustomize.py so the venv Python finds the build-tree bindings and
# their DLL dependencies on import.  Python 3.8+ on Windows no longer searches
# PATH for extension-module DLLs; os.add_dll_directory() is required instead.
file(WRITE "${_venv_dir}/@_venv_sitecustomize_subdir@/sitecustomize.py" [=[
import sys
import os

# Make 'import mitk' resolve to the build-tree package.
_mitk_sitearch = r'@Python3_SITEARCH@'
if _mitk_sitearch not in sys.path:
    sys.path.insert(0, _mitk_sitearch)

# Register each MITK runtime directory as a DLL search path (Python >= 3.8)
# and also prepend it to PATH for subprocess launches and older Python builds.
_mitk_dll_dirs = [
@_mitk_dll_paths_python_list@]
for _d in _mitk_dll_dirs:
    if os.path.exists(_d):
        if hasattr(os, 'add_dll_directory'):
            os.add_dll_directory(_d)
        os.environ['PATH'] = _d + os.pathsep + os.environ.get('PATH', '')
]=])

if(WIN32)
  set(_venv_python "${_venv_dir}/Scripts/python.exe")
else()
  set(_venv_python "${_venv_dir}/bin/python")
endif()

execute_process(
  COMMAND "${_venv_python}" -m pip install --quiet pytest
  RESULT_VARIABLE _pip_result
)
if(NOT _pip_result EQUAL 0)
  message(WARNING "Failed to install pytest in virtual environment at ${_venv_dir}")
endif()

message(STATUS "Virtual environment set up at ${_venv_dir}")
