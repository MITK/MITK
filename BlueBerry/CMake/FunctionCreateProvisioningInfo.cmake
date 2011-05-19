function(FunctionCreateProvisioningInfo info_var out_dir)

  set(out_var )
  foreach(plugin ${ARGN})
    ctkFunctionExtractOptionNameAndValue(${plugin} plugin_name_with_dirs plugin_value)
    string(REPLACE "/" ";" _tokens ${plugin_name_with_dirs})
    list(GET _tokens -1 plugin_name)
    string(REPLACE "." "_" plugin_target ${plugin_name})
    set(plugin_url "file://${out_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}${plugin_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    
    if(${${plugin_name_with_dirs}_option_name})
      set(out_var "${out_var}\nSTART ${plugin_url}")
    else()
      set(out_var "${out_var}\nSTOP ${plugin_url}")
    endif()
  endforeach()

  set(${info_var} ${out_var} PARENT_SCOPE)

endfunction() 
