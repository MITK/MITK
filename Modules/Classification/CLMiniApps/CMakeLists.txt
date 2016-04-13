option(BUILD_ClassificationMiniApps "Build commandline tools for classification" OFF)

if(BUILD_ClassificationMiniApps OR MITK_BUILD_ALL_APPS)


  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of miniapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( classificationminiapps
        RandomForestTraining^^MitkCLVigraRandomForest
        NativeHeadCTSegmentation^^MitkCLVigraRandomForest
        ManualSegmentationEvaluation^^MitkCLVigraRandomForest
        CLGlobalImageFeatures^^MitkCLUtilities
        CLMRNormalization^^MitkCLUtilities_MitkCLMRUtilities
        CLStaple^^MitkCLUtilities
        CLVoxelFeatures^^MitkCLUtilities
        CLDicom2Nrrd^^
        CLPolyToNrrd^^
        CLSimpleVoxelClassification^^MitkDataCollection_MitkCLVigraRandomForest
        CLVoxelClassification^^MitkDataCollection_MitkCLImportanceWeighting_MitkCLVigraRandomForest
        CLBrainMask^^MitkCLUtilities
    #    RandomForestPrediction^^MitkCLVigraRandomForest
    )

    foreach(classificationminiapps ${classificationminiapps})
      # extract mini app name and dependencies
      string(REPLACE "^^" "\\;" miniapp_info ${classificationminiapps})
      set(miniapp_info_list ${miniapp_info})
      list(GET miniapp_info_list 0 appname)
      list(GET miniapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitkFunctionCreateCommandLineApp(
        NAME ${appname}
        DEPENDS MitkCore MitkCLCore ${dependencies_list}
        PACKAGE_DEPENDS Vigra Qt5|Core
      )
    endforeach()

  # This mini app does not depend on MitkCLCore at all
  mitkFunctionCreateCommandLineApp(
    NAME CLImageConverter
    DEPENDS MitkCore ${dependencies_list}
  )

  mitkFunctionCreateCommandLineApp(
    NAME CLSurWeighting
    DEPENDS MitkCore MitkCLUtilities MitkDataCollection MitkCLImportanceWeighting  ${dependencies_list}
  )

  mitkFunctionCreateCommandLineApp(
    NAME CLImageCropper
    DEPENDS MitkCore MitkCLUtilities MitkAlgorithmsExt ${dependencies_list}
  )

endif()
