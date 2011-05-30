function(FunctionCreateProvisioningInfo)

  macro_parse_arguments(_PROV "OUTPUT;INCLUDE;PLUGIN_DIR;PLUGINS" "" ${ARGN})

  set(out_var )
  if(WIN32)
    set(file_url "file:///")
  else()
    set(file_url "file://")
  endif()
  
  foreach(incl ${_PROV_INCLUDE})
    set(out_var "${out_var}READ ${file_url}${incl}\n")
  endforeach()
  
  if(_PROV_INCLUDE)
    set(out_var "${out_var}\n")
  endif()
 
  foreach(plugin ${_PROV_PLUGINS})
    ctkFunctionExtractOptionNameAndValue(${plugin} plugin_name_with_dirs plugin_value)
    string(REPLACE "/" ";" _tokens ${plugin_name_with_dirs})
    list(GET _tokens -1 plugin_name)
    string(REPLACE "." "_" plugin_target ${plugin_name})
    
    set(plugin_url "${file_url}${_PROV_PLUGIN_DIR}/lib${plugin_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    
    if(${${plugin_name_with_dirs}_option_name})
      set(out_var "${out_var}START ${plugin_url}\n")
    #else()
    #  set(out_var "${out_var}STOP ${plugin_url}\n")
    endif()
  endforeach()

  set(${_PROV_OUTPUT} ${out_var} PARENT_SCOPE)

endfunction() 
