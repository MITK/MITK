#
# MITK specific install macro
#
# On Mac everything is installed for each bundle listed in MACOSX_BUNDLE_NAMES
# by replacing the DESTINATION parameter. Everything else is passed to the CMake INSTALL command
#
# Usage: MITK_INSTALL( )
#
MACRO(MITK_INSTALL)

  SET(ARGS ${ARGN})
 
  SET(install_directories "")
  LIST(FIND ARGS DESTINATION _destination_index)
  # SET(_install_DESTINATION "")
  IF(_destination_index GREATER -1)
    MESSAGE(SEND_ERROR "MITK_INSTALL macro must not be called with a DESTINATION parameter.")  
    ### This code was a try to replace a given DESTINATION
    #MATH(EXPR _destination_index ${_destination_index} + 1)
    #LIST(GET ARGS ${_destination_index} _install_DESTINATION)
    #STRING(REGEX REPLACE ^bin "" _install_DESTINATION ${_install_DESTINATION})
  ELSE()
    IF(NOT MACOSX_BUNDLE_NAMES)
      INSTALL(${ARGS} DESTINATION bin)
    ELSE()
      FOREACH(bundle_name ${MACOSX_BUNDLE_NAMES})
        INSTALL(${ARGS} DESTINATION ${bundle_name}.app/Contents/MacOS/${_install_DESTINATION})
      ENDFOREACH()
    ENDIF()
  ENDIF()

ENDMACRO()

# Fix _target_location
# This is used in several install macros
MACRO(_fixup_target)

  INSTALL(CODE "
    MACRO(gp_item_default_embedded_path_override item default_embedded_path_var)
      GET_FILENAME_COMPONENT(_item_name \"\${item}\" NAME)
      GET_FILENAME_COMPONENT(_item_path \"\${item}\" PATH)

      # We have to fix all path references to build trees for plugins

      IF(NOT _item_path MATCHES \"\${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}\")
        # item with relative path or embedded path pointing to some build dir
        SET(full_path \"full_path-NOTFOUND\")
        FILE (GLOB_RECURSE full_path \${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}/\${_item_name} )
        GET_FILENAME_COMPONENT(_item_path \"\${full_path}\" PATH)
      ENDIF()
      
      IF(_item_path STREQUAL \"\${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}/plugins\"
         OR _item_name MATCHES \"liborg\" # this is for legacy BlueBerry bundle support
        )
        # Only fix plugins
        MESSAGE(\"override: \${item}\")
        MESSAGE(\"found file: \${_item_path}/\${_item_name}\")
        IF(APPLE)
          STRING(REPLACE 
                 \${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir} 
                 @executable_path \${default_embedded_path_var} \"\${_item_path}\" )
        ELSE()
          SET(\${default_embedded_path_var} \"\${_item_path}\")
        ENDIF()
        MESSAGE(\"override result: \${\${default_embedded_path_var}}\")        
      ENDIF()
    ENDMACRO(gp_item_default_embedded_path_override)

    MACRO(gp_resolved_file_type_override file type)
      IF(NOT APPLE)
        GET_FILENAME_COMPONENT(_file_path \"\${file}\" PATH)
        GET_FILENAME_COMPONENT(_file_name \"\${file}\" NAME)
        IF(_file_path MATCHES \"^\${CMAKE_INSTALL_PREFIX}\")
          SET(\${type} \"local\")
        ENDIF()
        IF(_file_name MATCHES gdiplus)
            SET(\${type} \"system\")
        ENDIF(_file_name MATCHES gdiplus)
      ENDIF()
    ENDMACRO(gp_resolved_file_type_override)

    IF(NOT APPLE)
      IF(UNIX OR MINGW)
        MACRO(gp_resolve_item_override context item exepath dirs resolved_item_var resolved_var)
          IF(\${item} MATCHES \"blueberry_osgi\")
            GET_FILENAME_COMPONENT(_item_name \${item} NAME)
            SET(\${resolved_item_var} \"\${exepath}/plugins/\${_item_name}\")
            SET(\${resolved_var} 1)
          ENDIF()
        ENDMACRO()
      ENDIF()
    ENDIF()

    IF(\"${_install_GLOB_PLUGINS}\" STREQUAL \"TRUE\") 
      file(GLOB_RECURSE GLOBBED_BLUEBERRY_PLUGINS
        # glob for all blueberry bundles of this application
        \"\${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}/liborg*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    ENDIF()

    file(GLOB_RECURSE GLOBBED_QT_PLUGINS
      # glob for Qt plugins
      \"\${CMAKE_INSTALL_PREFIX}/${${_target_location}_qt_plugins_install_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")    
    
    # use custom version of BundleUtilities
    message(\"globbed plugins: \${GLOBBED_QT_PLUGINS} \${GLOBBED_BLUEBERRY_PLUGINS}\")
    SET(PLUGIN_DIRS)
    SET(PLUGINS ${_install_PLUGINS} \${GLOBBED_QT_PLUGINS} \${GLOBBED_BLUEBERRY_PLUGINS})
    if(PLUGINS)
      list(REMOVE_DUPLICATES PLUGINS)
    endif(PLUGINS)
    foreach(_plugin \${GLOBBED_BLUEBERRY_PLUGINS})
      get_filename_component(_pluginpath \${_plugin} PATH)
      list(APPEND PLUGIN_DIRS \${_pluginpath})
    endforeach(_plugin)
    set(DIRS ${DIRS})
    list(APPEND DIRS \${PLUGIN_DIRS})
    list(REMOVE_DUPLICATES DIRS)

    # use custom version of BundleUtilities
    SET(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH} )
    include(BundleUtilities)
    
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${_target_location}\" \"\${PLUGINS}\" \"\${DIRS}\")
  ")
ENDMACRO()
