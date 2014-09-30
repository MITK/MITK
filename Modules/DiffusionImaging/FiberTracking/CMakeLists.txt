
set(_module_deps MitkDiffusionCore MitkGraphAlgorithms)

mitk_check_module_dependencies(
  MODULES ${_module_deps}
  MISSING_DEPENDENCIES_VAR _missing_deps
)

# Enable OpenMP support
find_package(OpenMP)
if(NOT OPENMP_FOUND)
   message("OpenMP is not available.")
endif()
if(OPENMP_FOUND)
  message(STATUS "Found OpenMP.")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(NOT _missing_deps)

  set(lut_url http://mitk.org/download/data/FibertrackingLUT.tar.gz)
  set(lut_tarball ${CMAKE_CURRENT_BINARY_DIR}/FibertrackingLUT.tar.gz)
  message("Downloading FiberTracking LUT ${lut_url}...")
  file(DOWNLOAD ${lut_url} ${lut_tarball}
       EXPECTED_MD5 38ecb6d4a826c9ebb0f4965eb9aeee44
       TIMEOUT 60
       STATUS status
       SHOW_PROGRESS
      )

  list(GET status 0 status_code)
  list(GET status 1 status_msg)

  if(NOT status_code EQUAL 0)
    message(SEND_ERROR "${status_msg} (error code ${status_code})")
  else()
    message("done.")
  endif()

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
  message("Unpacking FiberTracking LUT tarball...")
  execute_process(COMMAND ${CMAKE_COMMAND} -E  tar xzf ../FibertrackingLUT.tar.gz
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
                  RESULT_VARIABLE result
                  ERROR_VARIABLE err_msg)
  if(result)
    message(SEND_ERROR "Unpacking FibertrackingLUT.tar.gz failed: ${err_msg}")
  else()
    message("done.")
  endif()

endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/GibbsTracking Algorithms/StochasticTracking IODataStructures IODataStructures/FiberBundleX IODataStructures/PlanarFigureComposite Interactions SignalModels Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${_module_deps}
  PACKAGE_DEPENDS ITK|ITKFFT ITK|ITKDiffusionTensorImage
  #WARNINGS_AS_ERRORS
)

if(MODULE_IS_ENABLED)
  add_subdirectory(Testing)
endif()
