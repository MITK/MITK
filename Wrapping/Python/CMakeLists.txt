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

mark_as_advanced(
    MITK_PYTHON_THREADS
)

set(libraries
  MitkCore
  ITKCommon
)

mitkSwigPrepareFiles(pyMITK MITKWRAP.i ${libraries})

set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword)

if(MITK_PYTHON_THREADS)
  list(APPEND CMAKE_SWIG_FLAGS -threads)
endif()

set(pyMITK_PACKAGE_DIR "${Python3_SITEARCH}/pyMITK")
string(REPLACE "\\" "/" pyMITK_PACKAGE_DIR "${pyMITK_PACKAGE_DIR}")
set(CMAKE_SWIG_OUTDIR "${pyMITK_PACKAGE_DIR}")

set(UseSWIG_TARGET_NAME_PREFERENCE STANDARD)
swig_add_library(pyMITK LANGUAGE python SOURCES MITKWRAP.i)
set_property(TARGET pyMITK PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Wrapping")
target_link_libraries(pyMITK Python3::Python ${libraries})
mitk_target_link_libraries_with_dynamic_lookup(${SWIG_MODULE_pyMITK_REAL_NAME} ${Python3_LIBRARIES})

set(lib_output_dir "${CMAKE_SWIG_OUTDIR}")

if(CMAKE_CONFIGURATION_TYPES)
  string(APPEND lib_output_dir "/$<CONFIG>/..")
endif()

set_target_properties(pyMITK PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${lib_output_dir}")

if(APPLE)
  set_target_properties(pyMITK PROPERTIES
    BUILD_RPATH "@loader_path/../../../../../lib;@loader_path/../../../../../../ep/lib"
    INSTALL_RPATH "@loader_path/../../../../../../MacOS"
  )
endif()

add_custom_command(
  TARGET pyMITK
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E rename pyMITK.py __init__.py
  WORKING_DIRECTORY "${pyMITK_PACKAGE_DIR}"
)

set(SWIG_RUNTIME_HEADER "${MITK_BINARY_DIR}/swigpyrun.h")
execute_process(COMMAND ${SWIG_EXECUTABLE} -python -external-runtime ${SWIG_RUNTIME_HEADER})
