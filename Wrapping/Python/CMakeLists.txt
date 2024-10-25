include(mitkTargetLinkLibrariesWithDynamicLookup)

project(MITK_Python)

set(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)

mitk_check_dynamic_lookup(MODULE
  SHARED
  MITK_UNDEFINED_SYMBOLS_ALLOWED
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option(MITK_PYTHON_THREADS "Enable threaded python usage by unlocking the GIL." ON )
option(MITK_PYTHON_EGG "Add building of python eggs to the dist target." OFF )
option(MITK_PYTHON_WHEEL "Add building of python wheels to the dist target." ON )

mark_as_advanced(
  MITK_PYTHON_THREADS
  MITK_PYTHON_EGG
  MITK_PYTHON_WHEEL
)

set(libraries
  MitkCore
  #MitkCLCore
  #MitkCLUtilities
  ITKCommon
  #MitkSegmentation
  #MitkMultilabel
  #MitkDICOM
  #MitkDICOMImageIO
  #MitkRT
)

if(MITK_USE_MatchPoint)
  list(APPEND libraries
     MitkMatchPointRegistration
  )
endif()



mitkSwigPrepareFiles(pyMITK MITKWRAP.i ${libraries})

set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword)

if(MITK_PYTHON_THREADS)
  list(APPEND CMAKE_SWIG_FLAGS -threads)
endif()

set(CMAKE_SWIG_OUTDIR ${MITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(UseSWIG_TARGET_NAME_PREFERENCE STANDARD)
swig_add_library(pyMITK LANGUAGE python SOURCES MITKWRAP.i)
set_property(TARGET pyMITK PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Wrapping")
target_link_libraries(pyMITK ${libraries})

mitk_target_link_libraries_with_dynamic_lookup(${SWIG_MODULE_pyMITK_REAL_NAME} ${Python3_LIBRARIES})
set_target_properties(pyMITK PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${MITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

execute_process(COMMAND ${SWIG_EXECUTABLE} -python -external-runtime)

if(DEFINED SKBUILD)
  #message("SKBuild exists")
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/pyMITK.py
      ${CMAKE_CURRENT_SOURCE_DIR}/Packaging/__init__.py
      #${MITK_DOC_FILES}
    DESTINATION pyMITK
    COMPONENT Runtime
  )
  install(
    TARGETS
      ${SWIG_MODULE_pyMITK_REAL_NAME}
    RUNTIME
      DESTINATION pyMITK
    LIBRARY
      DESTINATION pyMITK
    COMPONENT Runtime
  )
else()
  #message("SKBuild missing")
  include(LegacyPackaging.cmake)
endif()
