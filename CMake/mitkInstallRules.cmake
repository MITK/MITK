# Install MITK icon and logo

if(WIN32)
  install(FILES "${MITK_SOURCE_DIR}/mitk.ico" "${MITK_SOURCE_DIR}/mitk.bmp"
    DESTINATION bin)
endif()

# Install CppMicroServices - it uses usMacroCreateModule() (not mitk_create_module())
# so it is not tracked in MITK_MODULE_TARGETS, and US_NO_INSTALL disables its own
# install rules.

if(TARGET CppMicroServices)
  install(TARGETS CppMicroServices
    RUNTIME_DEPENDENCY_SET mitk_deps
    RUNTIME DESTINATION ${MITK_INSTALL_BINDIR}
    LIBRARY DESTINATION ${MITK_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION include/CppMicroServices EXCLUDE_FROM_ALL
    PRIVATE_HEADER DESTINATION include/CppMicroServices EXCLUDE_FROM_ALL)
endif()

# Install Python3 with MITK Python module

if(MITK_USE_Python3)
  if(APPLE)
    set(_python_dest "${MITK_INSTALL_FRAMEWORKSDIR}/Python.framework")
  else()
    set(_python_dest "python")
  endif()

  install(DIRECTORY "${MITK_BINARY_DIR}/python/"
    DESTINATION ${_python_dest}
    USE_SOURCE_PERMISSIONS)

  file(RELATIVE_PATH _rel_sitearch "${Python3_ROOT_DIR}" "${Python3_SITEARCH}")
  install(TARGETS mitk_python_bindings
    RUNTIME_DEPENDENCY_SET mitk_deps
    RUNTIME DESTINATION ${_python_dest}/${_rel_sitearch}/mitk
    LIBRARY DESTINATION ${_python_dest}/${_rel_sitearch}/mitk)
endif()

#-----------------------------------------------------------------------------
# Resolve and install ALL runtime dependencies in one pass.
#
# Every target that called install(TARGETS ... RUNTIME_DEPENDENCY_SET mitk_deps)
# — whether from mitk_create_module(), mitk_create_executable(),
# mitkFunctionCreateBlueBerryApplication(), mitkFunctionInstallCTKPlugin(),
# or this file — has been recorded. This single call resolves their transitive
# shared library dependencies and installs them.
#
# On macOS, Qt frameworks are excluded because qt_generate_deploy_app_script()
# handles them separately (deploying to Contents/Frameworks/). Without this
# exclusion, both mechanisms would deploy Qt, causing conflicts.
#-----------------------------------------------------------------------------

mitkFunctionGetLibrarySearchPaths(_search_dirs Release RELEASE)

install(RUNTIME_DEPENDENCY_SET mitk_deps
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
  RUNTIME DESTINATION ${MITK_INSTALL_BINDIR}
  LIBRARY DESTINATION ${MITK_INSTALL_FRAMEWORKSDIR}
  FRAMEWORK DESTINATION ${MITK_INSTALL_FRAMEWORKSDIR}
)

#-----------------------------------------------------------------------------
# Deploy Qt runtime dependencies (plugins, qt.conf, WebEngine resources).
#
# This runs AFTER install(RUNTIME_DEPENDENCY_SET) so that windeployqt can see
# all MITK DLLs in bin/ and correctly trace their transitive Qt dependencies.
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
