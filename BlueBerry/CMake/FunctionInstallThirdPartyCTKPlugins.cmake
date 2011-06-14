function(FunctionInstallThirdPartyCTKPlugins)

  set(install_directories "")
  if(NOT MACOSX_BUNDLE_NAMES)
    set(install_directories bin/plugins)
  else()
    foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
      list(APPEND install_directories ${bundle_name}.app/Contents/MacOS/plugins)
    endforeach(bundle_name)
  endif(NOT MACOSX_BUNDLE_NAMES)
  
  ctkMacroGetAllNonProjectTargetLibraries("${ARGN}" third_party_plugins)

  foreach(install_subdir ${install_directories})
    MACRO_INSTALL_CTK_PLUGIN(TARGETS ${third_party_plugins}
                             DESTINATION "${install_subdir}")
  endforeach()
  
endfunction()
