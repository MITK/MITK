function(mitkFunctionCreateWindowsBatchScript in out build_type)

  mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH ${build_type})

  set(VS_BUILD_TYPE ${build_type})

  configure_file(${in} ${out} @ONLY)

endfunction()