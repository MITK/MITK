#
# MITK specific cross plattform install macro
#
# Usage: MITK_INSTALL_TARGETS(target1 [target2] ....)
#
macro(MITK_INSTALL_TARGETS)
  MACRO_PARSE_ARGUMENTS(_install "TARGETS;EXECUTABLES;PLUGINS;LIBRARY_DIRS" "GLOB_PLUGINS" ${ARGN})
  list(APPEND _install_TARGETS ${_install_DEFAULT_ARGS})

  # TODO: how to supply the correct intermediate directory??
  # CMAKE_CFG_INTDIR is not expanded to actual values inside the install(CODE "...") macro ...
  set(intermediate_dir .)
  if(WIN32 AND NOT MINGW)
    set(intermediate_dir Release)
  endif()

  if(QT_LIBRARY_DIR MATCHES "^(/lib|/lib32|/lib64|/usr/lib|/usr/lib32|/usr/lib64|/usr/X11R6)(/.*)?$")
    set(_qt_is_system_qt 1)
  endif()

  foreach(_target ${_install_EXECUTABLES})

    get_target_property(_is_bundle ${_target} MACOSX_BUNDLE)

    set(_qt_plugins_install_dirs "")
    set(_qt_conf_install_dirs "")
    set(_target_locations "")

    if(APPLE)
      if(_is_bundle)
        set(_target_locations ${_target}.app)
        set(${_target_locations}_qt_plugins_install_dir ${_target}.app/Contents/MacOS)
        set(_bundle_dest_dir ${_target}.app/Contents/MacOS)
        set(_qt_plugins_for_current_bundle ${_target}.app/Contents/MacOS)
        set(_qt_conf_install_dirs ${_target}.app/Contents/Resources)
        install(TARGETS ${_target} BUNDLE DESTINATION . )
      else()
        if(NOT MACOSX_BUNDLE_NAMES)
          set(_qt_conf_install_dirs bin)
          set(_target_locations bin/${_target})
          set(${_target_locations}_qt_plugins_install_dir bin)
          install(TARGETS ${_target} RUNTIME DESTINATION bin)
        else()
          foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
            list(APPEND _qt_conf_install_dirs ${bundle_name}.app/Contents/Resources)
            set(_current_target_location ${bundle_name}.app/Contents/MacOS/${_target})
            list(APPEND _target_locations ${_current_target_location})
            set(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS)
            message( "  set(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS) ")

            install(TARGETS ${_target} RUNTIME DESTINATION ${bundle_name}.app/Contents/MacOS/)
          endforeach()
        endif()
      endif()
    else()
      set(_target_locations bin/${_target}${CMAKE_EXECUTABLE_SUFFIX})
      set(${_target_locations}_qt_plugins_install_dir bin)
      set(_qt_conf_install_dirs bin)
      install(TARGETS ${_target} RUNTIME DESTINATION bin)
    endif()

    foreach(_target_location ${_target_locations})
      if(NOT _qt_is_system_qt)

        if(QT_PLUGINS_DIR)
          if(WIN32)
            install(DIRECTORY "${QT_PLUGINS_DIR}"
                    DESTINATION ${${_target_location}_qt_plugins_install_dir}
                    CONFIGURATIONS Release
                    FILES_MATCHING REGEX "[^4d]4?${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   )

            install(DIRECTORY "${QT_PLUGINS_DIR}"
                    DESTINATION ${${_target_location}_qt_plugins_install_dir}
                    CONFIGURATIONS Debug
                    FILES_MATCHING REGEX "d4?${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   )
          else(WIN32)
            # install everything, see bug 7143
            install(DIRECTORY "${QT_PLUGINS_DIR}"
                    DESTINATION ${${_target_location}_qt_plugins_install_dir}
                    FILES_MATCHING REGEX "${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   )

          endif(WIN32)

        endif()
      endif()
      _fixup_target()
    endforeach()

    if(NOT _qt_is_system_qt)

      #--------------------------------------------------------------------------------
      # install a qt.conf file
      # this inserts some cmake code into the install script to write the file
      set(_qt_conf_plugin_install_prefix .)
      if(APPLE)
        set(_qt_conf_plugin_install_prefix ./MacOS)
      endif()

      foreach(_qt_conf_install_dir ${_qt_conf_install_dirs})
        install(CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${_qt_conf_install_dir}/qt.conf\" \"
[Paths]
Prefix=${_qt_conf_plugin_install_prefix}
\")")
      endforeach()

    endif()

  endforeach()

endmacro()
