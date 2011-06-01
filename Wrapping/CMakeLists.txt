PROJECT(mitkWrap)
cmake_minimum_required(VERSION 2.6)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/mitkWrapSetup.cmake)

# Find wrapping language API libraries.
IF(MITK_USE_Python)
  INCLUDE(${CMAKE_ROOT}/Modules/FindPythonLibs.cmake)
  FIND_PROGRAM(PYTHON_EXECUTABLE
    NAMES python python2.3 python2.2 python2.1 python2.0 python1.6 python1.5 
    PATHS
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]
    )
  MARK_AS_ADVANCED(PYTHON_EXECUTABLE)
ENDIF(MITK_USE_Python)

MARK_AS_ADVANCED(MITK_USE_Python)
MARK_AS_ADVANCED(CABLE_INDEX)
MARK_AS_ADVANCED(CSWIG)
MARK_AS_ADVANCED(GCCXML)

# set a variable to determine if 
# the CSwig directory should be used
SET(MITK_CSWIG_DIR 0)
IF(MITK_USE_Python)
  SET(MITK_CSWIG_DIR 1)
ENDIF(MITK_USE_Python)

IF(MITK_CSWIG_DIR)
  SUBDIRS(CSwig)
ENDIF(MITK_CSWIG_DIR)
