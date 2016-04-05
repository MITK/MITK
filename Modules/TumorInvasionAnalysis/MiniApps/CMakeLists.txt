option(BUILD_TumorInvasionAnalysisMiniApps "Build commandline tools for tumor invasion analysis" OFF)

if(BUILD_TumorInvasionAnalysisMiniApps OR MITK_BUILD_ALL_APPS)


  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of miniapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( tumorinvasionminiapps
        BasicDistanceMaps^^
        MriNormalization^^
        MriOtsuNormalization^^
        #Resampling^^
        TumorResponseEvaluationTool^^
        HistogramAdaption^^
        TumorProgressionMapping^^
        ConnectednessFeatureMaps^^
        TumorInvasionAnalysisTool^^
    )

    foreach(tumorinvasionminiapps ${tumorinvasionminiapps})
      # extract mini app name and dependencies
      string(REPLACE "^^" "\\;" miniapp_info ${tumorinvasionminiapps})
      set(miniapp_info_list ${miniapp_info})
      list(GET miniapp_info_list 0 appname)
      list(GET miniapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkCLCore MitkDataCollection MitkTumorInvasionAnalysis ${dependencies_list}
      )
    endforeach()

endif()
