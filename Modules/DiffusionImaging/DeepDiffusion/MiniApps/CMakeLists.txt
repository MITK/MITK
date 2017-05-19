option(BUILD_DeepDiffusionMiniApps "Build MITK diffusion commandline tools with deep learning support." OFF)

if(BUILD_DeepDiffusionMiniApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  # list of diffusion miniapps
  # if an app requires additional dependencies
  # they are added after a "^^" and separated by "_"
  set( deepdiffusionminiapps
  Caffe2Test^^
  )

  foreach(miniapp ${deepdiffusionminiapps})
    # extract mini app name and dependencies
    string(REPLACE "^^" "\\;" miniapp_info ${miniapp})
    set(miniapp_info_list ${miniapp_info})
    list(GET miniapp_info_list 0 appname)
    list(GET miniapp_info_list 1 raw_dependencies)
    string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
    set(dependencies_list ${dependencies})

    mitkFunctionCreateCommandLineApp(
      NAME ${appname}
      DEPENDS MitkCore MitkDiffusionCore ${dependencies_list}
      PACKAGE_DEPENDS ITK Eigen Caffe2 OpenCV
    )
  endforeach()

endif()
