option(BUILD_ModelFitMiniApps "Build commandline tools for the ModelFit module" OFF)

if(BUILD_ModelFitMiniApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )
    # list of miniapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( miniapps
    GenericFittingMiniApp^^
    PixelDumpMiniApp^^
    Fuse3Dto4DImageMiniApp^^
    )

    foreach(miniapp ${miniapps})
      # extract mini app name and dependencies
      string(REPLACE "^^" "\\;" miniapp_info ${miniapp})
      set(miniapp_info_list ${miniapp_info})
      list(GET miniapp_info_list 0 appname)
      list(GET miniapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkModelFit ${dependencies_list}
      )
    endforeach()

endif(BUILD_ModelFitMiniApps OR MITK_BUILD_ALL_APPS)
