function(mitkFunctionCreateWindowsBatchScript in out build_type)
  if(GDCM_DIR)
    set(GDCM_BIN_DIR "${GDCM_DIR}/bin/${build_type}")
  else()
    set(GDCM_BIN_DIR)
  endif()
  
  configure_file(${in} ${out} @ONLY)
endfunction()