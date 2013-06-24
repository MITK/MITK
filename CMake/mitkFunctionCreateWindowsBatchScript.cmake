function(mitkFunctionCreateWindowsBatchScript in out build_type)

  mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH ${build_type})

  string(REPLACE "%VS_BUILD_TYPE%" "${build_type}" MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH}")

  configure_file(${in} ${out} @ONLY)

endfunction()