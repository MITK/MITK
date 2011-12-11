#!
#! \brief Create a provisioning file
#!
#! \param FILE <provisioning-file> (required) An absolute filename for
#!                                 the new provisioning file.
#! \param INCLUDE <file-list> (optional) A list of additional provisioning files
#!                            which should be included.
#! \param PLUGINS <plugin-list> (optional) A list of target names for which provisioning
#!                              entries should be created. The entries must be valid targets or
#!                              be of the form [subdir/]target_name:OFF (this is the same form as
#!                              passed to the ctkMacroSetupPlugins macro) If the list is empty,
#!                              all known plug-in targets (external or internal) are considered.
#! \param EXCLUDE_PLUGINS <plugin-list> (optional) A list of plug-in symbolic names which should be excluded
#!                                      from the provisioning entries.
#!
#! This function creates a provisioning file which can be used to provision a BlueBerry
#! application. The syntax of entries in the file is
#! \code
#! (READ|INSTALL|START) <file-url>
#! \endcode
#! READ includes the file at <file-url> and interprets it as a provisioning file, INSTALL installs <file-url>,
#! and START installs and starts <file-url> as a plug-in in the framework.
#!
#! <p>
#! For example the following provisioning file instructs the BlueBerry framework to read the entries in
#! a file called SomeApp.provisioning and subsequently INSTALL and START the plug-in com.mycompany.plugin
#! \code
#! READ file:///opt/etc/SomeApp.provisioning
#! START file:///opt/mycompany/plugins/libcom_mycompany_plugin.so
#! \endcode
#!
#! <p>
#! An example invocation of this macro may look like:
#! \code
#! set(_my_prov_file "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MyApp.provisioning")
#! set(_my_plugins
#!   com.mycompany.plugin
#!   org.mitk.gui.qt.extapplication
#! )
#! FunctionCreateProvisioningFile(FILE ${_my_prov_file} PLUGINS ${_my_plugins})
#! \endcode
#!
#! \note This function will automatically create entries for all plug-in
#! dependencies of the specified plug-ins.
#!
function(FunctionCreateProvisioningFile)

  macro_parse_arguments(_PROV "FILE;INCLUDE;PLUGINS;EXCLUDE_PLUGINS;PLUGIN_DIR" "" ${ARGN})
  
  if(_PROV_PLUGIN_DIR)
    message(WARNING "The PLUGIN_DIR argument is no longer supported. Either use FunctionCreateProvisioningFile_legacy or adapt your CMake function call.")
  endif()
  
  if(NOT _PROV_FILE)
    message(SEND_ERROR "FILE argument must not be empty")
    return()
  endif()
  
  set(out_var )
  set(out_var_install )
  if(WIN32)
    set(file_url "file:///")
  else()
    set(file_url "file://")
  endif()
  
  # Include other provisioning files
  foreach(incl ${_PROV_INCLUDE})
    get_filename_component(incl_filename "${incl}" NAME)
    set(out_var "${out_var}READ ${file_url}${incl}\n")
    set(out_var_install "${out_var_install}READ ${file_url}@EXECUTABLE_DIR/${incl_filename}\n")
  endforeach()
  
  if(_PROV_INCLUDE)
    set(out_var "${out_var}\n")
    set(out_var_install "${out_var_install}\n")
  endif()
  
  set(_plugin_list )
  if(_PROV_PLUGINS)
    foreach(_plugin ${_PROV_PLUGINS})
      string(REPLACE "." "_" _plugin_target ${_plugin})
      list(APPEND _plugin_list ${_plugin_target})
    endforeach()
  else()
    # Fill the _plugin_list variable with external and internal plug-in target names.
    ctkFunctionGetAllPluginTargets(_plugin_list)
  endif()

  set(_exclude_targets )
  if(_PROV_EXCLUDE_PLUGINS)
    # Convert the plug-in symbolic names to valid target names
    foreach(_exclude_entry ${_PROV_EXCLUDE_PLUGINS})
      string(REPLACE "." "_" _exclude_target ${_exclude_entry})
      list(APPEND _exclude_targets ${_exclude_target})
    endforeach()
    list(REMOVE_ITEM _plugin_list ${_exclude_targets})
  endif()

  # Go through the list of plug-ins
  foreach(plugin ${_plugin_list})
    set(_plugin_target)
    if(TARGET ${plugin})
      # The entry already is a valid target (either imported or declared in the current project)
      set(_plugin_target ${plugin})
    else()
      # Check if the entry if of the form "Some/Dir/org.my.plugin:OPTION"
      ctkFunctionExtractOptionNameAndValue(${plugin} plugin_name_with_dirs plugin_value)
      string(REPLACE "/" ";" _tokens ${plugin_name_with_dirs})
      list(GET _tokens -1 plugin_name)
      string(REPLACE "." "_" _plugin_target_name ${plugin_name})
      if(TARGET ${_plugin_target_name})
        # Check if the extracted last directory entry is a valid target
        set(_plugin_target ${_plugin_target_name})
      endif()
    endif()
    
    if(_plugin_target)
      # We got a valid target, either imported or from this project.
      set(_plugin_location)
      get_target_property(_is_imported ${_plugin_target} IMPORTED)
      if(_is_imported)
        get_target_property(_plugin_location ${_plugin_target} IMPORTED_LOCATION)
        if(NOT _plugin_location)
          get_target_property(_plugin_configs ${_plugin_target} IMPORTED_CONFIGURATIONS)
          foreach(_plugin_config ${_plugin_configs})
            get_target_property(_plugin_location ${_plugin_target} IMPORTED_LOCATION_${_plugin_config})
            if(_plugin_location)
              if(CMAKE_CONFIGURATION_TYPES)
                # Strip the last directory and filename
                string(REGEX REPLACE "(.*)/[^/]*/[^/]*$" "\\1" _plugin_location "${_plugin_location}")
              else()
                # Just strip the filename
                get_filename_component(_plugin_location "${_plugin_location}" PATH)
              endif()
              break()
            endif()
          endforeach()
        endif()
      else()
        if(WIN32)
          get_target_property(_plugin_location ${_plugin_target} RUNTIME_OUTPUT_DIRECTORY)
        else()
          get_target_property(_plugin_location ${_plugin_target} LIBRARY_OUTPUT_DIRECTORY)
        endif()
      endif()

      set(plugin_url "${file_url}${_plugin_location}/lib${_plugin_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set(plugin_url_install "${file_url}@EXECUTABLE_DIR/plugins/lib${_plugin_target}${CMAKE_SHARED_LIBRARY_SUFFIX}")

      set(out_var "${out_var}START ${plugin_url}\n")
      set(out_var_install "${out_var_install}START ${plugin_url_install}\n")
    else()
      #message(WARNING "Ignoring unknown plug-in target \"${plugin}\" for provisioning.")
    endif()

  endforeach()

  file(WRITE ${_PROV_FILE} "${out_var}")
  file(WRITE ${_PROV_FILE}.install "${out_var_install}")

endfunction() 

function(FunctionCreateProvisioningFile_legacy)

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
