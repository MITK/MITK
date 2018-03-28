option(BUILD_DiffusionCoreCmdApps "Build commandline tools for diffusion" OFF)

if(BUILD_DiffusionCoreCmdApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of diffusion cmdapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( diffusioncorecmdapps
    ImageResampler^^
    CopyGeometry^^
    Registration^^
    DiffusionDICOMLoader^^
    ResampleGradients^^
    ShToOdfImage^^
    DImp^^
    DReg^^
    )

    foreach(diffusioncorecmdapp ${diffusioncorecmdapps})
      # extract cmd app name and dependencies
      string(REPLACE "^^" "\\;" cmdapp_info ${diffusioncorecmdapp})
      set(cmdapp_info_list ${cmdapp_info})
      list(GET cmdapp_info_list 0 appname)
      list(GET cmdapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkDiffusionCore ${dependencies_list}
        PACKAGE_DEPENDS ITK
      )
    endforeach()

endif()
