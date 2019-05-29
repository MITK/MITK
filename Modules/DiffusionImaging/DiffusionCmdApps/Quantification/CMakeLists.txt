option(BUILD_DiffusionQuantificationCmdApps "Build commandline tools for diffusion quantification (IVIM, ADC, ...)" OFF)

if(BUILD_DiffusionQuantificationCmdApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of diffusion cmdapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( diffusionQuantificationcmdapps
    DiffusionIndices^^
    QballReconstruction^^
    TensorReconstruction^^
    DiffusionKurtosisFit^^
    MultishellMethods^^
    )

    foreach(diffusionQuantificationcmdapp ${diffusionQuantificationcmdapps})
      # extract cmd app name and dependencies
      string(REPLACE "^^" "\\;" cmdapp_info ${diffusionQuantificationcmdapp})
      set(cmdapp_info_list ${cmdapp_info})
      list(GET cmdapp_info_list 0 appname)
      list(GET cmdapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkDiffusionCmdApps ${dependencies_list}
        PACKAGE_DEPENDS
      )
    endforeach()
    
endif()
