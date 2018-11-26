function(mitkFunctionCreateWindowsBatchScript in out build_type)

  if(${build_type} STREQUAL "debug")
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH ${build_type} DEBUG)
  else()
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH ${build_type} RELEASE)
  endif()

  set(VS_BUILD_TYPE ${build_type})

  configure_file(${in} ${out} @ONLY)

endfunction()