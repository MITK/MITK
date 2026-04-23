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
# Deploy Qt runtime dependencies (plugins, qt.conf, WebEngine resources).
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
# Reset RUNPATH of bundled CTK core libraries on Linux.
#
# CTK has INSTALL_COMMAND "" in CMakeExternals/CTK.cmake, so CTK_DIR points
# directly at the build tree. install(RUNTIME_DEPENDENCY_SET) copies libCTK*.so*
# in as a transitive dependency of MITK plug-ins but does not rewrite RPATHs,
# so the bundled CTK libraries retain absolute build-host paths in their
# RUNPATH (e.g. the Qt install prefix, and the CTK build dir itself as the
# first entry — which shadows $ORIGIN). That makes the bundle non-relocatable:
# on a machine that happens to have a different CTK build tree or Qt install
# at the baked-in paths, the loader follows them into foreign libraries and
# triggers ABI mismatches.
#
# Reset RUNPATH to "$ORIGIN" so these libraries resolve peers (Qt, ITK, DCMTK,
# etc.) from the same bin/ directory. Skip symlinks — only the real .so files
# carry RPATH, and file(RPATH_SET) on a symlink is meaningless.
#
# Windows: PE has no RPATH. macOS: macdeployqt rewrites library references
# during Qt deployment, so Mach-O references are already flattened.
#-----------------------------------------------------------------------------

if(LINUX)
  foreach(_bindir IN LISTS MITK_INSTALL_BINDIR)
    install(CODE "
      file(GLOB _mitk_ctk_libs \"\${CMAKE_INSTALL_PREFIX}/${_bindir}/libCTK*.so*\")
      foreach(_lib IN LISTS _mitk_ctk_libs)
        if(NOT IS_SYMLINK \"\${_lib}\")
          file(RPATH_SET FILE \"\${_lib}\" NEW_RPATH \"\$ORIGIN\")
        endif()
      endforeach()
    ")
  endforeach()
endif()
