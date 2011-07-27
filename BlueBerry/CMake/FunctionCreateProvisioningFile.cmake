function(FunctionCreateProvisioningFile)

  macro_parse_arguments(_PROV "FILE;INCLUDE;PLUGIN_DIR;PLUGINS" "" ${ARGN})

  set(out_var )
  set(out_var_install )
  if(WIN32)
    set(file_url "file:///")
  else()
    set(file_url "file://")
  endif()
  
  foreach(incl ${_PROV_INCLUDE})
    get_filename_component(incl_filename "${incl}" NAME)
    set(out_var "${out_var}READ ${file_url}${incl}\n")
    set(out_var_install "${out_var_install}READ ${file_url}@EXECUTABLE_DIR/${incl_filename}\n")
  endforeach()
  
  if(_PROV_INCLUDE)
    set(out_var "${out_var}\n")
    set(out_var_install "${out_var_install}\n")
  endif()

  string(REPLACE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "@EXECUTABLE_DIR" _PROV_PLUGIN_DIR_install ${_PROV_PLUGIN_DIR})
 
  foreach(plugin ${_PROV_PLUGINS})
    ctkFunctionExtractOptionNameAndValue(${plugin} plugin_name_with_dirs plugin_value)
    string(REPLACE "/" ";" _tokens ${plugin_name_with_dirs})
    list(GET _tokens -1 plugin_name)
    string(REPLACE "." "_" plugin_target ${plugin_name})
    
    set(plugin_url "${file_url}${_PROV_PLUGIN_DIR}/lib${plugin_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(plugin_url_install "${file_url}${_PROV_PLUGIN_DIR_install}/lib${plugin_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    
    if(${${plugin_name_with_dirs}_option_name})
      set(out_var "${out_var}START ${plugin_url}\n")
      set(out_var_install "${out_var_install}START ${plugin_url_install}\n")
    #else()
    #  set(out_var "${out_var}STOP ${plugin_url}\n")
    #  set(out_var_install "${out_var_install}STOP ${plugin_url_install}\n")
    endif()
  endforeach()

  file(WRITE ${_PROV_FILE} "${out_var}")
  file(WRITE ${_PROV_FILE}.install "${out_var_install}")

endfunction() 
