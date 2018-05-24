#
# This module finds the languages supported by MITK, and
# present the option to enable support.
#

#
# This script is based on SimpleITK scripts.
#

option(WRAP_DEFAULT "The default initial value for wrapping a language when it is detected on the system." OFF)
mark_as_advanced(WRAP_DEFAULT)

#
# Macro to set "_QUIET" and "_QUIET_LIBRARY" based on the first
# argument being defined and true, to either REQUIRED or QUIET.
#
macro(set_QUIET var)
  if ( DEFINED  ${var} AND ${var} )
    set( _QUIET "REQUIRED" )
  else()
    set( _QUIET "QUIET" )
  endif()
  if ( SITK_UNDEFINED_SYMBOLS_ALLOWED )
    set( _QUIET_LIBRARY "QUIET" )
  else()
    set( _QUIET_LIBRARY ${_QUIET} )
  endif()
endmacro()

#
# Setup the option for each language
#

#-----------------------------------------------------------
# Python

set_QUIET( WRAP_PYTHON )
find_package ( PythonInterp ${_QUIET})
if ( PYTHONINTERP_FOUND )
  find_package ( PythonLibs ${PYTHON_VERSION_STRING} EXACT ${_QUIET_LIBRARY} )
else ()
  find_package ( PythonLibs ${_QUIET_LIBRARY} )
endif()

if ( PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND
    AND (PYTHON_VERSION_STRING VERSION_EQUAL PYTHONLIBS_VERSION_STRING) )
  set( WRAP_PYTHON_DEFAULT ${WRAP_DEFAULT} )
else()
  set( WRAP_PYTHON_DEFAULT OFF )
endif()

option( WRAP_PYTHON "Wrap Python" ${WRAP_PYTHON_DEFAULT} )

if ( WRAP_PYTHON AND PYTHON_VERSION_STRING VERSION_LESS 2.7 )
  message( WARNING "Python version less than 2.7: \"${PYTHON_VERSION_STRING}\"." )
endif()

if ( WRAP_PYTHON )
  list( APPEND SITK_LANGUAGES_VARS
    PYTHON_DEBUG_LIBRARY
    PYTHON_EXECUTABLE
    PYTHON_LIBRARY
    PYTHON_INCLUDE_DIR
    )
# Debian "jessie" has this additional variable required to match
# python versions.
  if(PYTHON_INCLUDE_DIR2)
    list( APPEND SITK_LANGUAGES_VARS
      PYTHON_INCLUDE_DIR2
      )
  endif()
endif ()
