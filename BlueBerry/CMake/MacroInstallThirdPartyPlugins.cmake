MACRO(MACRO_INSTALL_THIRDPARTY_PLUGINS plugins_dir install_dir)

  SET(install_directories "")
  IF(NOT MACOSX_BUNDLE_NAMES)
    SET(install_directories bin)
  ELSE()
    FOREACH(bundle_name ${MACOSX_BUNDLE_NAMES})
      LIST(APPEND install_directories ${bundle_name}.app/Contents/MacOS)
    ENDFOREACH(bundle_name)
  ENDIF(NOT MACOSX_BUNDLE_NAMES)
  
  FILE(GLOB_RECURSE plugins
       # glob for all blueberry bundles in this directory
      "${plugins_dir}/liborg*${CMAKE_SHARED_LIBRARY_SUFFIX}")

  FOREACH(plugin ${plugins})
    
    FILE(RELATIVE_PATH _toplevel_dir "${plugins_dir}" "${plugin}")
    STRING(REGEX REPLACE "([^/]+)/.*$" "\\1" _plugin_base_dir ${_toplevel_dir})
  
    FOREACH(install_subdir ${install_directories})
      MACRO_INSTALL_PLUGIN("${plugins_dir}/${_plugin_base_dir}" 
                           DESTINATION "${install_subdir}/${install_dir}")
    ENDFOREACH()
  ENDFOREACH()
ENDMACRO()