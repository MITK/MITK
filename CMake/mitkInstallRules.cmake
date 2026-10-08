# Install MITK icon and logo

if(WIN32)
  install(FILES "${MITK_SOURCE_DIR}/mitk.ico" "${MITK_SOURCE_DIR}/mitk.bmp"
    DESTINATION bin)
endif()

# Install Python3 with MITK Python module

if(MITK_USE_Python3)
  foreach(_bindir _fwdir _depset IN ZIP_LISTS MITK_INSTALL_BINDIR MITK_INSTALL_FRAMEWORKSDIR MITK_RUNTIME_DEPENDENCY_SETS)
    if(APPLE)
      set(_python_dest "${_fwdir}/Python.framework")
    else()
      set(_python_dest "python")
    endif()

    install(DIRECTORY "${MITK_BINARY_DIR}/python/"
      DESTINATION ${_python_dest}
      USE_SOURCE_PERMISSIONS)

    file(RELATIVE_PATH _rel_sitearch "${Python3_ROOT_DIR}" "${Python3_SITEARCH}")
    install(TARGETS mitk_python_bindings
      RUNTIME_DEPENDENCY_SET ${_depset}
      RUNTIME DESTINATION ${_python_dest}/${_rel_sitearch}/mitk
      LIBRARY DESTINATION ${_python_dest}/${_rel_sitearch}/mitk)

    # The install RPATH of MITK's binaries is relative to the directory of the
    # executables and does not reach their libraries from inside the Python
    # installation. Without entries of its own, the module resolves its
    # dependencies only while an MITK executable is the host process, not in
    # the bundled interpreter. The wheel configuration sets its own RPATH.
    if(NOT WIN32 AND NOT MITK_BUILD_CONFIGURATION STREQUAL "PythonWheel")
      set(_module_dir "${_python_dest}/${_rel_sitearch}/mitk")
      file(RELATIVE_PATH _to_bindir "/${_module_dir}" "/${_bindir}")
      if(APPLE)
        file(RELATIVE_PATH _to_fwdir "/${_module_dir}" "/${_fwdir}")
        set_property(TARGET mitk_python_bindings PROPERTY INSTALL_RPATH
          "@loader_path/${_to_bindir}" "@loader_path/${_to_fwdir}")
      else()
        set_property(TARGET mitk_python_bindings PROPERTY INSTALL_RPATH
          "$ORIGIN/${_to_bindir}")
      endif()
    endif()
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Resolve and install runtime dependencies.
#
# Every target that called install(TARGETS ... RUNTIME_DEPENDENCY_SET <set>)
# has been recorded. Each dependency set is resolved independently to its own
# bundle's directories. On Windows/Linux there is a single set ("mitk_deps")
# resolving to "bin/". On macOS there is one set per bundle.
#
# On macOS, Qt frameworks are excluded because qt_generate_deploy_app_script()
# handles them separately (deploying to Contents/Frameworks/). Without this
# exclusion, both mechanisms would deploy Qt, causing conflicts.
#-----------------------------------------------------------------------------

mitkFunctionGetLibrarySearchPaths(_search_dirs Release RELEASE)

foreach(_bindir _fwdir _depset IN ZIP_LISTS MITK_INSTALL_BINDIR MITK_INSTALL_FRAMEWORKSDIR MITK_RUNTIME_DEPENDENCY_SETS)
  install(RUNTIME_DEPENDENCY_SET ${_depset}
    PRE_EXCLUDE_REGEXES
      "^api-ms-"
      "^ext-ms-"
      "^msvcp[0-9]+"
      "^vcruntime[0-9]+"
      "^concrt[0-9]+"
      "^vcomp[0-9]+"
      "^ucrtbase"
      "Qt[A-Z].*\\.framework"   # macOS: pre-exclude; unresolved @rpath Qt refs (e.g. VTK) are fatal before POST_EXCLUDE runs
    POST_EXCLUDE_REGEXES
      "[/\\\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\]"
      "^/usr/lib"
      "^/lib"
      "^/System"
      "python3[0-9]+[.]"
      ".*/plugins/.*"
      ".*Qt[A-Z].*\\.framework.*"   # Qt frameworks — handled by qt_generate_deploy_app_script() on macOS
      ".*/Qt[A-Z].*\\.dylib$"       # Qt dylibs (non-framework form) — same reason
    DIRECTORIES ${_search_dirs}
    RUNTIME DESTINATION ${_bindir}
    LIBRARY DESTINATION ${_fwdir}
    FRAMEWORK DESTINATION ${_fwdir}
  )
endforeach()

#-----------------------------------------------------------------------------
# Install the Crashpad handler.
#
# The handler is spawned at runtime, not linked, so RUNTIME_DEPENDENCY_SET
# cannot discover it. The sentry backend resolves it next to the application
# executable, hence one copy per bundle bin directory.
#-----------------------------------------------------------------------------

if(MITK_USE_sentry AND MITK_EXTERNAL_PROJECT_PREFIX)
  # Renamed to match the handler_path the facility sets (see Modules/CrashHandling).
  foreach(_bindir IN LISTS MITK_INSTALL_BINDIR)
    install(PROGRAMS "${MITK_CRASH_HANDLER_EXECUTABLE}"
      DESTINATION ${_bindir}
      RENAME ${MITK_CRASH_HANDLER_NAME})
  endforeach()

  install(FILES "${MITK_EXTERNAL_PROJECT_PREFIX}/src/sentry/LICENSE"
    DESTINATION share/licenses/sentry-native)
  install(FILES "${MITK_EXTERNAL_PROJECT_PREFIX}/src/sentry/external/crashpad/LICENSE"
    DESTINATION share/licenses/sentry-native
    RENAME LICENSE.crashpad)
endif()

#-----------------------------------------------------------------------------
# Deploy Qt runtime dependencies (plugins, qt.conf).
#
# This runs AFTER install(RUNTIME_DEPENDENCY_SET) so that windeployqt can see
# all MITK DLLs in bin/ and correctly trace their transitive Qt dependencies.
# On macOS, macdeployqt runs per-app via qt_generate_deploy_app_script(),
# so each bundle gets its own Qt deployment.
#-----------------------------------------------------------------------------

get_property(_mitk_executable_targets GLOBAL PROPERTY MITK_EXECUTABLE_TARGETS)

if(MITK_USE_Qt6 AND _mitk_executable_targets)
  foreach(_mitk_executable_target ${_mitk_executable_targets})
    get_target_property(_no_install ${_mitk_executable_target} NO_INSTALL)
    if(_no_install)
      continue()
    endif()
    get_target_property(_deploy_qt ${_mitk_executable_target} MITK_DEPLOY_QT)
    if(_deploy_qt)
      mitkFunctionDeployQt(${_mitk_executable_target})
    endif()
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Drop absolute LC_RPATH entries of the bundled binaries on macOS.
#
# Libraries copied in from the superbuild or a package manager, such as CTK
# (used from its build tree) or Homebrew and MacPorts libraries, keep their
# absolute LC_RPATH entries, and macdeployqt only removes the ones it used.
# dyld searches them before the bundle, so on a machine that has libraries at
# these paths, the bundle loads foreign ones, for example a second Qt. Replace
# them by an entry relative to the binary that reaches Contents/Frameworks.
#
# This is the last step that changes binaries, so it also re-signs those
# whose signature the changes before invalidated.
#-----------------------------------------------------------------------------

if(APPLE AND MACOSX_BUNDLE_NAMES)
  foreach(_bundle IN LISTS MACOSX_BUNDLE_NAMES)
    install(CODE "
      set(_mitk_macho_tools \"${MITK_SOURCE_DIR}/CMake/mitkMachOTools.cmake\")
      set(_mitk_bundle \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_bundle}.app\")
    ")
    install(CODE [[
      include("${_mitk_macho_tools}")
      mitk_macho_files(_binaries "${_mitk_bundle}")
      foreach(_binary IN LISTS _binaries)
        mitk_macho_read("${_binary}" RPATHS _rpaths)
        set(_args "")
        foreach(_rpath IN LISTS _rpaths)
          if(NOT _rpath MATCHES "^@")
            list(APPEND _args -delete_rpath "${_rpath}")
          endif()
        endforeach()
        if(NOT _args)
          continue()
        endif()
        get_filename_component(_dir "${_binary}" DIRECTORY)
        file(RELATIVE_PATH _to_frameworks "${_dir}" "${_mitk_bundle}/Contents/Frameworks")
        set(_frameworks_rpath "@loader_path/${_to_frameworks}")
        string(REGEX REPLACE "/$" "" _frameworks_rpath "${_frameworks_rpath}")
        if(NOT _frameworks_rpath IN_LIST _rpaths)
          list(APPEND _args -add_rpath "${_frameworks_rpath}")
        endif()
        execute_process(COMMAND install_name_tool ${_args} "${_binary}" RESULT_VARIABLE _result)
        if(NOT _result EQUAL 0)
          message(FATAL_ERROR "install_name_tool failed on ${_binary}")
        endif()
      endforeach()
      mitk_macho_resign_invalid(${_binaries})
    ]])
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Strip MITK's own libraries again after the Qt deployment.
#
# On Linux, qt_deploy_runtime_dependencies() resolves the dependencies of the
# build-tree executable and copies them into the package, which replaces the
# stripped copies of MITK's own libraries with the unstripped build-tree
# files. Excluding them from that resolution is not an option: the exclusion
# also stops the search below them, so Qt modules reached only through MITK
# libraries would lose their plugins. CMake strips only what install(TARGETS)
# installs, hence the second pass. The build-tree copies also carry the
# absolute build RUNPATH, so the install RUNPATH is restored as well.
#-----------------------------------------------------------------------------

if(LINUX AND CMAKE_STRIP AND US_DEFAULT_RESOURCE_MODE STREQUAL "LINK")
  string(REPLACE ";" ":" _mitk_install_rpath "${CMAKE_INSTALL_RPATH}")
  install(CODE "
    if(CMAKE_INSTALL_DO_STRIP)
      file(GLOB _mitk_libraries \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/*.so\")
      foreach(_library \${_mitk_libraries})
        get_filename_component(_name \"\${_library}\" NAME)
        set(_installed \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MITK_INSTALL_BINDIR}/\${_name}\")
        if(EXISTS \"\${_installed}\")
          execute_process(COMMAND \"${CMAKE_STRIP}\" \"\${_installed}\")
          file(RPATH_SET FILE \"\${_installed}\" NEW_RPATH \"${_mitk_install_rpath}\")
        endif()
      endforeach()
    endif()
  ")
endif()

#-----------------------------------------------------------------------------
# Drop absolute RUNPATH entries of the bundled binaries on Linux.
#
# install(RUNTIME_DEPENDENCY_SET), the Qt deployment and the Crashpad handler
# install copy third-party binaries without rewriting their RUNPATH. The external projects carry the
# Qt install prefix in theirs (see SuperBuild.cmake), and CTK, which is used
# from its build tree, carries that build tree as well. On a machine that has
# libraries at these paths, the loader follows them into foreign libraries,
# so the package is not relocatable.
#
# Keep only the $ORIGIN-relative entries, or $ORIGIN alone if none are left:
# every bundled binary resolves its peers from the same bin/ directory.
#
# Windows: PE has no RPATH. macOS: macdeployqt rewrites library references
# during Qt deployment, so Mach-O references are already flattened.
#-----------------------------------------------------------------------------

if(LINUX)
  foreach(_bindir IN LISTS MITK_INSTALL_BINDIR)
    install(CODE "set(_mitk_bindir \"${_bindir}\")")
    install(CODE [[
      file(GLOB _mitk_binaries "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${_mitk_bindir}/*")
      foreach(_binary IN LISTS _mitk_binaries)
        if(IS_SYMLINK "${_binary}" OR IS_DIRECTORY "${_binary}")
          continue()
        endif()
        set(_runpath "")
        set(_rpath "")
        set(_error "")
        file(READ_ELF "${_binary}" RUNPATH _runpath RPATH _rpath CAPTURE_ERROR _error)
        if(_error)
          continue()
        endif()
        set(_current "${_runpath}")
        if(NOT _current)
          set(_current "${_rpath}")
        endif()
        if(NOT _current)
          continue()
        endif()
        string(REPLACE ":" ";" _entries "${_current}")
        set(_relative_entries "")
        foreach(_entry IN LISTS _entries)
          string(FIND "${_entry}" "$ORIGIN" _position)
          if(_position EQUAL 0)
            list(APPEND _relative_entries "${_entry}")
          endif()
        endforeach()
        list(REMOVE_DUPLICATES _relative_entries)
        if(NOT _relative_entries)
          set(_relative_entries "$ORIGIN")
        endif()
        string(REPLACE ";" ":" _new "${_relative_entries}")
        if(NOT _new STREQUAL _current)
          file(RPATH_SET FILE "${_binary}" NEW_RPATH "${_new}")
        endif()
      endforeach()
    ]])
  endforeach()
endif()
