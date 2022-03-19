# Version 2.8.1 is the minimum requirement for this script.
# this is lower than the general minimum requirement.
#cmake_minimum_required ( VERSION 2.8.1 FATAL_ERROR )

include(mitkTargetLinkLibrariesWithDynamicLookup)

project( MITK_Python )

set(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)

mitk_check_dynamic_lookup(MODULE
  SHARED
  MITK_UNDEFINED_SYMBOLS_ALLOWED
  )

#
# Find the necessary libraries etc..
#

if ( MITK_UNDEFINED_SYMBOLS_ALLOWED )
  set( _QUIET_LIBRARY "QUIET" )
else()
  set( _QUIET_LIBRARY "REQUIRED" )
endif()

find_package ( PythonInterp REQUIRED )
find_package ( PythonLibs ${_QUIET_LIBRARY} )
include_directories ( ${CMAKE_CURRENT_SOURCE_DIR} )
#
# Options
#
option ( MITK_PYTHON_THREADS "Enable threaded python usage by unlocking the GIL." ON )
mark_as_advanced( MITK_PYTHON_THREADS )
option ( MITK_PYTHON_EGG "Add building of python eggs to the dist target." OFF )
mark_as_advanced( MITK_PYTHON_EGG )
option ( MITK_PYTHON_WHEEL "Add building of python wheels to the dist target." ON )
mark_as_advanced( MITK_PYTHON_WHEEL )

# Prepare the SWIG-File, i.e. especially add necessary include folders
mitkSwigPrepareFiles(pyMITK MITK.i "MitkCore;MitkCLCore;MitkCLUtilities;ITKCommon;MitkMatchPointRegistration;MitkSegmentation;MitkMultilabel;MitkDICOM;MitkDICOMImageIO;MitkRT")

# Add additional SWIG Parameters
# These parameters depend on the target language
set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword )
if( MITK_PYTHON_THREADS )
  set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -threads)
endif()
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

# Create the actual SWIG project
swig_add_module(pyMITK python MITK.i )

mitkSwigAddLibraryDependencies(pyMITK "MitkCore;MitkCLCore;MitkCLUtilities;ITKCommon;MitkMatchPointRegistration;MitkSegmentation;MitkMultilabel;MitkDICOM;MitkDICOMImageIO;MitkRT")
mitk_target_link_libraries_with_dynamic_lookup(${SWIG_MODULE_pyMITK_REAL_NAME} ${PYTHON_LIBRARIES})


if(DEFINED SKBUILD)
  message(WARNING "SKBuild exists")
  # Currently this installation
  install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/pyMITK.py
      ${CMAKE_CURRENT_SOURCE_DIR}/Packaging/__init__.py
    #  ${MITK_DOC_FILES}
    DESTINATION pyMITK
    COMPONENT Runtime
    )

  install(TARGETS ${SWIG_MODULE_pyMITK_REAL_NAME}
    RUNTIME DESTINATION pyMITK
    LIBRARY DESTINATION pyMITK
    COMPONENT Runtime
    )
else()
  message(WARNING "SKBuild missing")
  include(LegacyPackaging.cmake)
endif()
