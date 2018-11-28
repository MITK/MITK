
set(_module_deps MitkDiffusionCore MitkGraphAlgorithms MitkCLVigraRandomForest)

mitk_check_module_dependencies(
  MODULES ${_module_deps}
  MISSING_DEPENDENCIES_VAR _missing_deps
)

if(NOT _missing_deps)

  set(lut_url http://mitk.org/download/data/FibertrackingLUT.tar.gz)
  set(lut_tarball ${CMAKE_CURRENT_BINARY_DIR}/FibertrackingLUT.tar.gz)
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
  endif()

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
  execute_process(COMMAND ${CMAKE_COMMAND} -E  tar xzf ../FibertrackingLUT.tar.gz
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
                  RESULT_VARIABLE result
                  ERROR_VARIABLE err_msg)
  if(result)
    message(SEND_ERROR "Unpacking FibertrackingLUT.tar.gz failed: ${err_msg}")
  endif()

endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Fiberfox Fiberfox/SignalModels Fiberfox/Sequences Algorithms Algorithms/TrackingHandlers Algorithms/ClusteringMetrics Algorithms/GibbsTracking Algorithms/StochasticTracking IODataStructures IODataStructures/FiberBundle IODataStructures/PlanarFigureComposite Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${_module_deps}
  PACKAGE_DEPENDS PUBLIC ITK|ITKFFT ITK|ITKDiffusionTensorImage Vigra HDF5 OpenMP|OpenMP_CXX
)

if(MODULE_IS_ENABLED)
  add_subdirectory(Testing)
endif()
