project(mitkWrap)
cmake_minimum_required(VERSION 2.6)

include(${CMAKE_CURRENT_SOURCE_DIR}/mitkWrapSetup.cmake)

# Find wrapping language API libraries.
if(MITK_USE_Python)
  include(${CMAKE_ROOT}/Modules/FindPythonLibs.cmake)
  find_program(PYTHON_EXECUTABLE
    NAMES python python2.3 python2.2 python2.1 python2.0 python1.6 python1.5 
    PATHS
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]
    )
  mark_as_advanced(PYTHON_EXECUTABLE)
endif(MITK_USE_Python)

mark_as_advanced(MITK_USE_Python)
mark_as_advanced(CABLE_INDEX)
mark_as_advanced(CSWIG)
mark_as_advanced(GCCXML)

# set a variable to determine if 
# the CSwig directory should be used
set(MITK_CSWIG_DIR 0)
if(MITK_USE_Python)
  set(MITK_CSWIG_DIR 1)
endif(MITK_USE_Python)

if(MITK_CSWIG_DIR)
  subdirs(CSwig)
endif(MITK_CSWIG_DIR)
