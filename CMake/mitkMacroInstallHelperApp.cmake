#! MITK specific cross plattform install macro
#!
#! Usage: MITK_INSTALL_HELPER_APP(target1 [target2] ....)
#!
macro(MITK_INSTALL_HELPER_APP)
  MACRO_PARSE_ARGUMENTS(_install "TARGETS;EXECUTABLES;PLUGINS;LIBRARY_DIRS" "GLOB_PLUGINS" ${ARGN})
  list(APPEND _install_TARGETS ${_install_DEFAULT_ARGS})

  # TODO: how to supply to correct intermediate directory??
  # CMAKE_CFG_INTDIR is not expanded to actual values inside the install(CODE "...") macro ...
  set(intermediate_dir .)
  if(WIN32 AND NOT MINGW)
    set(intermediate_dir Release)
  endif()

  mitkFunctionGetLibrarySearchPaths(DIRS ${intermediate_dir})

  if(APPLE)
    list(APPEND DIRS "/usr/lib")
  endif(APPLE)

  if(QT_LIBRARY_DIR MATCHES "^(/lib|/lib32|/lib64|/usr/lib|/usr/lib32|/usr/lib64|/usr/X11R6)(/.*)?$")
    set(_qt_is_system_qt 1)
  endif()

  foreach(_target ${_install_EXECUTABLES})

    set(_qt_plugins_install_dirs "")
    set(_qt_conf_install_dirs "")
    set(_target_locations "")

    get_filename_component(_target_name ${_target} NAME)

    if(APPLE)
      if(NOT MACOSX_BUNDLE_NAMES)
        set(_qt_conf_install_dirs bin)
        set(_target_locations bin/${_target_name})
        set(${_target_locations}_qt_plugins_install_dir bin)
        install(PROGRAMS ${_target} DESTINATION bin)
      else()
        foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
          list(APPEND _qt_conf_install_dirs ${bundle_name}.app/Contents/Resources)
          set(_current_target_location ${bundle_name}.app/Contents/MacOS/${_target_name})
          list(APPEND _target_locations ${_current_target_location})
          set(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS)

          install(PROGRAMS ${_target} DESTINATION ${bundle_name}.app/Contents/MacOS/)
        endforeach()
      endif(NOT MACOSX_BUNDLE_NAMES)
    else()
      set(_target_locations bin/${_target_name})
      set(${_target_locations}_qt_plugins_install_dir bin)
      set(_qt_conf_install_dirs bin)
      install(PROGRAMS ${_target} DESTINATION bin)
      if(UNIX AND NOT WIN32)
        # Remove the rpath from helper applications. We assume that all dependencies
        # are installed into the same location as the helper application.
        install(CODE "file(RPATH_REMOVE
                           FILE \"\${CMAKE_INSTALL_PREFIX}/${_target_location}\")")
      endif()
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
    endforeach(_target_location)

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

endmacro(MITK_INSTALL_HELPER_APP)
