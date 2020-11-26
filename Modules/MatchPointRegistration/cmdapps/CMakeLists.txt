option(BUILD_MatchPointCmdApps "Build commandline tools for the MatchPoint module" OFF)

if(BUILD_MatchPointCmdApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )
    # list of CmdApps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( cmdapps
    StitchImagesMiniApp^^
    )

    foreach(cmdapp ${cmdapps})
      # extract cmd name and dependencies
      string(REPLACE "^^" "\\;" cmdapp_info ${cmdapp})
      set(cmdapp_info_list ${cmdapp_info})
      list(GET cmdapp_info_list 0 appname)
      list(GET cmdapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkMatchPointRegistration ${dependencies_list}
      )
    endforeach()

endif(BUILD_MatchPointCmdApps OR MITK_BUILD_ALL_APPS)
