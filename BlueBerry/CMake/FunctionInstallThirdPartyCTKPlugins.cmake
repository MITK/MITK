#! \brief Install external CTK plug-ins.
#!
#! Use this macro to call MACRO_INSTALL_CTK_PLUGIN() for all known
#! or given external CTK plug-ins. The arguments given until a known argument name
#! are treated as plug-in target names. If none are given, all known
#! external plug-ins will be installed.
#!
#! This macro supports multiple MacOSX Bundles by checking the variable
#! MACOSX_BUNDLE_NAMES. If it exists, each plug-in will be installed
#! in each given bundle (under <bundle_name>/Contents/MacOS/plugins), otherwise
#! it will be installed in "bin/plugins".
#!
#! Typical usage (will install all known external plug-ins):
#! \code
#! FunctionInstallThirdPartyCTKPlugins()
#! \endcode
#!
#! To install only the org.commontk.eventadmin plug-in for example, call:
#! \code
#! FunctionInstallThirdPartyCTKPlugins(org.commontk.eventadmin)
#! \endcode
#!
#! \note If you provide specific plug-in names as arguments, you must
#! make sure that your list includes the transitive closure of all plug-in
#! dependencies. Use the ctkFunctionGetPluginDependencies() macro to obtain
#! all dependencies of a specified set of plug-ins.
#!
function(FunctionInstallThirdPartyCTKPlugins)

  macro_parse_arguments(_INSTALL "EXCLUDE" "" ${ARGN})

  set(install_directories "")
  if(NOT MACOSX_BUNDLE_NAMES)
    set(install_directories bin/plugins)
  else()
    foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
      list(APPEND install_directories ${bundle_name}.app/Contents/MacOS/plugins)
    endforeach(bundle_name)
  endif(NOT MACOSX_BUNDLE_NAMES)

  if(_INSTALL_DEFAULT_ARGS)
    set(_ctk_plugins )
    foreach(_ctk_plugin ${_INSTALL_DEFAULT_ARGS})
      string(REPLACE "." "_" _plugin_target ${_ctk_plugin})
      list(APPEND _ctk_plugins ${_ctk_plugin})
    endforeach()
  else()
    ctkFunctionGetAllPluginTargets(_ctk_plugins)
  endif()

  if(_INSTALL_EXCLUDE)
    set(_exclude_list )
    foreach(_exclude ${_INSTALL_EXCLUDE})
      string(REPLACE "." "_" _exclude_target ${_exclude})
      list(APPEND _exclude_list ${_exclude_target})
    endforeach()
    list(REMOVE_ITEM _ctk_plugins ${_INSTALL_EXCLUDE})
  endif()

  ctkMacroGetAllNonProjectTargetLibraries("${_ctk_plugins}" _third_party_ctk_plugins)
  if(_third_party_ctk_plugins)
    foreach(install_subdir ${install_directories})
      MACRO_INSTALL_CTK_PLUGIN(TARGETS ${_third_party_ctk_plugins}
                               DESTINATION "${install_subdir}")
    endforeach()
  endif()
  
endfunction()
