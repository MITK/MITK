#
# MITK specific cross plattform install macro
#
# Usage: MITK_INSTALL_TARGETS(target1 [target2] ....)
#
MACRO(MITK_INSTALL_TARGETS)
  MACRO_PARSE_ARGUMENTS(_install "TARGETS;EXECUTABLES;PLUGINS;LIBRARY_DIRS" "GLOB_PLUGINS" ${ARGN})
  LIST(APPEND _install_TARGETS ${_install_DEFAULT_ARGS})

  # TODO: how to supply the correct intermediate directory??
  # CMAKE_CFG_INTDIR is not expanded to actual values inside the INSTALL(CODE "...") macro ...
  SET(intermediate_dir )
  IF(WIN32 AND NOT MINGW)
    SET(intermediate_dir Release)
  ENDIF()
  
  SET(DIRS 
    ${MITK_VTK_LIBRARY_DIRS}/${intermediate_dir}
    ${MITK_ITK_LIBRARY_DIRS}/${intermediate_dir}
    ${QT_LIBRARY_DIR} 
    ${QT_LIBRARY_DIR}/../bin
    ${MITK_BINARY_DIR}/bin/${intermediate_dir}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${intermediate_dir}
    )

  FOREACH(_lib_dir ${_install_LIBRARY_DIRS})
    LIST(APPEND DIRS ${_lib_dir}/${intermediate_dir})
  ENDFOREACH()
  
  IF(MITK_USE_Boost AND MITK_USE_Boost_LIBRARIES AND NOT MITK_USE_SYSTEM_Boost)
    LIST(APPEND DIRS ${Boost_LIBRARY_DIRS})
  ENDIF()
  IF(GDCM_DIR)
    LIST(APPEND DIRS ${GDCM_DIR}/bin/${intermediate_dir})
  ENDIF()
  IF(OpenCV_DIR)
    LIST(APPEND DIRS ${OpenCV_DIR}/bin/${intermediate_dir})
  ENDIF()
  IF(MITK_USE_BLUEBERRY)
    LIST(APPEND DIRS ${CTK_RUNTIME_LIBRARY_DIRS}/${intermediate_dir})
  ENDIF()

  LIST(REMOVE_DUPLICATES DIRS)

  if(QT_LIBRARY_DIR MATCHES "^(/lib/|/lib32/|/lib64/|/usr/lib/|/usr/lib32/|/usr/lib64/|/usr/X11R6/)")
    set(_qt_is_system_qt 1)
  endif()

  FOREACH(_target ${_install_EXECUTABLES})

    GET_TARGET_PROPERTY(_is_bundle ${_target} MACOSX_BUNDLE)

    SET(_qt_plugins_install_dirs "")
    SET(_qt_conf_install_dirs "")
    SET(_target_locations "")

    IF(APPLE)
      IF(_is_bundle)
        SET(_target_locations ${_target}.app)
        SET(${_target_locations}_qt_plugins_install_dir ${_target}.app/Contents/MacOS)
        SET(_bundle_dest_dir ${_target}.app/Contents/MacOS)
        SET(_qt_plugins_for_current_bundle ${_target}.app/Contents/MacOS)
        SET(_qt_conf_install_dirs ${_target}.app/Contents/Resources)
        INSTALL(TARGETS ${_target} BUNDLE DESTINATION . )
      ELSE()
        IF(NOT MACOSX_BUNDLE_NAMES)
          SET(_qt_conf_install_dirs bin)
          SET(_target_locations bin/${_target})
          SET(${_target_locations}_qt_plugins_install_dir bin)
          INSTALL(TARGETS ${_target} RUNTIME DESTINATION bin)
        ELSE()
          FOREACH(bundle_name ${MACOSX_BUNDLE_NAMES})
            LIST(APPEND _qt_conf_install_dirs ${bundle_name}.app/Contents/Resources)
            SET(_current_target_location ${bundle_name}.app/Contents/MacOS/${_target})
            LIST(APPEND _target_locations ${_current_target_location})
            SET(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS)
            MESSAGE( "  SET(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS) ")

            INSTALL(TARGETS ${_target} RUNTIME DESTINATION ${bundle_name}.app/Contents/MacOS/)
          ENDFOREACH()
        ENDIF()
      ENDIF()
    ELSE()
      SET(_target_locations bin/${_target}${CMAKE_EXECUTABLE_SUFFIX})
      SET(${_target_locations}_qt_plugins_install_dir bin)
      SET(_qt_conf_install_dirs bin)
      INSTALL(TARGETS ${_target} RUNTIME DESTINATION bin)
    ENDIF() 

    FOREACH(_target_location ${_target_locations})
      IF(NOT _qt_is_system_qt)

        IF(QT_PLUGINS_DIR)
          IF(WIN32)
            INSTALL(DIRECTORY "${QT_PLUGINS_DIR}"
                    DESTINATION ${${_target_location}_qt_plugins_install_dir}
                    CONFIGURATIONS Release
                    FILES_MATCHING REGEX "[^4d]4?${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   )

            INSTALL(DIRECTORY "${QT_PLUGINS_DIR}"
                    DESTINATION ${${_target_location}_qt_plugins_install_dir}
                    CONFIGURATIONS Debug
                    FILES_MATCHING REGEX "d4?${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   )
          ELSE(WIN32)
            # install everything, see bug 7143
            INSTALL(DIRECTORY "${QT_PLUGINS_DIR}"
                    DESTINATION ${${_target_location}_qt_plugins_install_dir}
                    FILES_MATCHING REGEX "${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   )
 
          ENDIF(WIN32)

        ENDIF()
      ENDIF()
      _fixup_target()
    ENDFOREACH()

    IF(NOT _qt_is_system_qt)

      #--------------------------------------------------------------------------------
      # install a qt.conf file
      # this inserts some cmake code into the install script to write the file
      SET(_qt_conf_plugin_install_prefix .)
      IF(APPLE)
        SET(_qt_conf_plugin_install_prefix ./MacOS)
      ENDIF()
      
      FOREACH(_qt_conf_install_dir ${_qt_conf_install_dirs})
        INSTALL(CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${_qt_conf_install_dir}/qt.conf\" \"
[Paths]
Prefix=${_qt_conf_plugin_install_prefix}
\")")
      ENDFOREACH()

    ENDIF()

  ENDFOREACH()

ENDMACRO()
