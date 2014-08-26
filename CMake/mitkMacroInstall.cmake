#
# MITK specific install macro
#
# On Mac everything is installed for each bundle listed in MACOSX_BUNDLE_NAMES
# by replacing the DESTINATION parameter. Everything else is passed to the CMake INSTALL command
#
# Usage: MITK_INSTALL( )
#
macro(MITK_INSTALL)

  set(ARGS ${ARGN})

  set(install_directories "")
  list(FIND ARGS DESTINATION _destination_index)
  # set(_install_DESTINATION "")
  if(_destination_index GREATER -1)
    message(SEND_ERROR "MITK_INSTALL macro must not be called with a DESTINATION parameter.")
    ### This code was a try to replace a given DESTINATION
    #math(EXPR _destination_index ${_destination_index} + 1)
    #list(GET ARGS ${_destination_index} _install_DESTINATION)
    #string(REGEX REPLACE ^bin "" _install_DESTINATION ${_install_DESTINATION})
  else()
    if(NOT MACOSX_BUNDLE_NAMES)
      install(${ARGS} DESTINATION bin)
    else()
      foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
        install(${ARGS} DESTINATION ${bundle_name}.app/Contents/MacOS/${_install_DESTINATION})
      endforeach()
    endif()
  endif()

endmacro()

# Fix _target_location
# This is used in several install macros
macro(_fixup_target)

  if(NOT intermediate_dir)
    if(WIN32 AND NOT MINGW)
      set(intermediate_dir Release)
    else()
      set(intermediate_dir .)
    endif()
  endif()

  set(_python_libs )
  set(_python_dirs )
  if(MITK_USE_Python)
    include(mitkFunctionInstallPython)
    mitkFunctionInstallPython(_python_libs _python_dirs "${_bundle_dest_dir}")
  endif()

  mitkFunctionGetLibrarySearchPaths(_search_paths ${intermediate_dir})

  install(CODE "

    set(_bundle_dest_dir \"${_bundle_dest_dir}\")
    if(_bundle_dest_dir)
      set(_bin_path \"\${CMAKE_INSTALL_PREFIX}/\${_bundle_dest_dir}\")
    else()
      set(_bin_path \"\${CMAKE_INSTALL_PREFIX}/bin\")
    endif()

    macro(gp_item_default_embedded_path_override item default_embedded_path_var)
      get_filename_component(_item_name \"\${item}\" NAME)
      get_filename_component(_item_path \"\${item}\" PATH)

      # We have to fix all path references to build trees for plugins

      if(NOT _item_path MATCHES \"\${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}\")
        # item with relative path or embedded path pointing to some build dir
        set(full_path \"full_path-NOTFOUND\")
        file(GLOB_RECURSE full_path \${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}/\${_item_name} )
        list(LENGTH full_path full_path_length)
        if(full_path_length GREATER 1)
          list(GET full_path 0 full_path)
        endif()
        get_filename_component(_item_path \"\${full_path}\" PATH)
      endif()

      set(_plugins_path \"\${_bin_path}/plugins\")

      if(_item_path STREQUAL _plugins_path
         OR (_item_path MATCHES \"\${_plugins_path}/\" AND _item_name MATCHES \"liborg\") # this is for legacy BlueBerry bundle support
        )
        # Only fix plugins
        message(\"override: \${item}\")
        message(\"found file: \${_item_path}/\${_item_name}\")
        if(APPLE)
          string(REPLACE
                 \${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}
                 @executable_path \${default_embedded_path_var} \"\${_item_path}\" )
        else()
          set(\${default_embedded_path_var} \"\${_item_path}\")
        endif()
        message(\"override result: \${\${default_embedded_path_var}}\")
      endif()
    endmacro(gp_item_default_embedded_path_override)

    macro(gp_resolved_file_type_override file type)
      if(NOT APPLE)
        get_filename_component(_file_path \"\${file}\" PATH)
        get_filename_component(_file_name \"\${file}\" NAME)
        if(_file_path MATCHES \"^\${CMAKE_INSTALL_PREFIX}\")
          set(\${type} \"local\")
        endif()
        if(_file_name MATCHES gdiplus)
            set(\${type} \"system\")
        endif(_file_name MATCHES gdiplus)
      endif()
    endmacro(gp_resolved_file_type_override)

    if(NOT APPLE)
      macro(gp_resolve_item_override context item exepath dirs resolved_item_var resolved_var)
        if(\${item} MATCHES \"blueberry_osgi\")
          get_filename_component(_item_name \${item} NAME)
          set(\${resolved_item_var} \"\${exepath}/plugins/\${_item_name}\")
          set(\${resolved_var} 1)
        endif()
      endmacro()
    endif()

    if(\"${_install_GLOB_PLUGINS}\" STREQUAL \"TRUE\")
      set(GLOBBED_PLUGINS )
      set(_bb_osgi_lib \"\${_bin_path}/liborg_blueberry_osgi${CMAKE_SHARED_LIBRARY_SUFFIX}\")
      if(EXISTS \"\${_bb_osgi_lib}\")
        list(APPEND GLOBBED_PLUGINS \"\${_bb_osgi_lib}\")
      endif()

      # Iterate over all sub-directories which contain plug-ins
      # (BlueBerry plug-ins, Qt plug-ins, and auto-load modules)
      file(GLOB _children \"\${_bin_path}/*\")
      foreach(_child \${_children})
        if(IS_DIRECTORY \${_child})
          set(_plugins )
          set(_modules )
          file(GLOB_RECURSE _plugins \"\${_child}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
          if(_plugins)
            list(APPEND GLOBBED_PLUGINS \${_plugins})
          endif()
          # Now glob for all modules which might have a different extensions.
          # E.g. on MacOS plugins could have a .dylib extension as well as a .so extension
          if(NOT \"${CMAKE_SHARED_MODULE_SUFFIX}\" STREQUAL \"\" AND NOT \"${CMAKE_SHARED_MODULE_SUFFIX}\" STREQUAL \"${CMAKE_SHARED_LIBRARY_SUFFIX}\")
            file(GLOB_RECURSE _modules \"\${_child}/*${CMAKE_SHARED_MODULE_SUFFIX}\")
          endif()
          if(_modules)
            list(APPEND GLOBBED_PLUGINS \${_modules})
          endif()
        endif()
      endforeach()
    endif()

    set(PLUGINS )
    foreach(_plugin ${_install_PLUGINS} \${GLOBBED_PLUGINS})
      get_filename_component(_plugin_realpath \${_plugin} REALPATH)
      list(APPEND PLUGINS \${_plugin_realpath})
    endforeach()

    foreach(_py_lib ${_python_libs})
      list(APPEND PLUGINS \"\${_bin_path}/\${_py_lib}\")
    endforeach()

    if(PLUGINS)
      list(REMOVE_DUPLICATES PLUGINS)
    endif(PLUGINS)
    message(\"globbed plugins: \${PLUGINS}\")

    set(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH} )

    set(DIRS \"${_search_paths}\")

    set(_additional_search_paths ${_install_LIBRARY_DIRS})
    if(_additional_search_paths)
      set(DIRS \"\${DIRS};\${_additional_search_paths}\")
    endif()
    foreach(_plugin \${PLUGINS})
      get_filename_component(_pluginpath \${_plugin} PATH)
      list(APPEND DIRS \"\${_pluginpath}\")
    endforeach(_plugin)

    foreach(_dir ${_python_dirs})
      list(APPEND DIRS \"\${_dir}\")
    endforeach()

    list(REMOVE_DUPLICATES DIRS)

    # use custom version of BundleUtilities
    include(BundleUtilities)

    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${_target_location}\" \"\${PLUGINS}\" \"\${DIRS}\")
  ")
endmacro()
