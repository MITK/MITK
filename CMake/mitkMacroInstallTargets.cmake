#
# MITK specific cross plattform install macro
#
# Usage: MITK_INSTALL_TARGETS(target1 [target2] ....)
#
macro(MITK_INSTALL_TARGETS)
  cmake_parse_arguments(_install "GLOB_PLUGINS" "" "TARGETS;EXECUTABLES;PLUGINS;LIBRARY_DIRS" ${ARGN})
  list(APPEND _install_TARGETS ${_install_DEFAULT_ARGS})

  # TODO: how to supply the correct intermediate directory??
  # CMAKE_CFG_INTDIR is not expanded to actual values inside the install(CODE "...") macro ...
  set(intermediate_dir .)
  if(WIN32)
    set(intermediate_dir Release)
  endif()

  foreach(_target ${_install_EXECUTABLES})
    get_target_property(_is_bundle ${_target} MACOSX_BUNDLE)

    set(_qt_conf_install_dirs "")
    set(_target_locations "")

    if(APPLE)
      if(_is_bundle)
        set(_target_locations ${_target}.app)
        set(${_target_locations}_qt_plugins_install_dir ${_target}.app/Contents/MacOS)
        set(_bundle_dest_dir ${_target}.app/Contents/MacOS)
        set(_qt_conf_install_dirs ${_target}.app/Contents/Resources)
        install(TARGETS ${_target} BUNDLE DESTINATION . )
      else()
        if(NOT MACOSX_BUNDLE_NAMES)
          set(_qt_conf_install_dirs bin)
          set(_target_locations bin/${_target})
          install(TARGETS ${_target} RUNTIME DESTINATION bin)
        else()
          foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
            list(APPEND _qt_conf_install_dirs ${bundle_name}.app/Contents/Resources)
            set(_current_target_location ${bundle_name}.app/Contents/MacOS/${_target})
            list(APPEND _target_locations ${_current_target_location})
            install(TARGETS ${_target} RUNTIME DESTINATION ${bundle_name}.app/Contents/MacOS/)
          endforeach()
        endif()
      endif()
    else()
      set(_target_locations bin/${_target}${CMAKE_EXECUTABLE_SUFFIX})
      set(_qt_conf_install_dirs bin)
      install(TARGETS ${_target} RUNTIME DESTINATION bin)
    endif()

    foreach(_target_location ${_target_locations})
      _fixup_target()
    endforeach()

    #--------------------------------------------------------------------------------
    # install a qt.conf file
    # this inserts some cmake code into the install script to write the file
    if(MITK_USE_Qt5)
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
