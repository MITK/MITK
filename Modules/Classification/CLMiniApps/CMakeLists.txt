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
        CLGlobalImageFeatures^^MitkCore_MitkCLUtilities
        CLMRNormalization^^MitkCore_MitkCLUtilities_MitkCLMRUtilities
        CLStaple^^MitkCore_MitkCLUtilities
        CLVoxelFeatures^^MitkCore_MitkCLUtilities
        CLDicom2Nrrd^^MitkCore
        CLPolyToNrrd^^MitkCore
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

      mitk_create_executable(${appname}
      DEPENDS MitkCore MitkCLCore ${dependencies_list}
      PACKAGE_DEPENDS ITK Qt4|QtCore Qt5|Core Vigra
      CPP_FILES ${appname}.cpp mitkCommandLineParser.cpp
      )

      if(EXECUTABLE_IS_ENABLED)

        # On Linux, create a shell script to start a relocatable application
        if(UNIX AND NOT APPLE)
          install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ${EXECUTABLE_TARGET}.sh)
        endif()

        get_target_property(_is_bundle ${EXECUTABLE_TARGET} MACOSX_BUNDLE)

        if(APPLE)
         if(_is_bundle)
           set(_target_locations ${EXECUTABLE_TARGET}.app)
           set(${_target_locations}_qt_plugins_install_dir ${EXECUTABLE_TARGET}.app/Contents/MacOS)
           set(_bundle_dest_dir ${EXECUTABLE_TARGET}.app/Contents/MacOS)
           set(_qt_plugins_for_current_bundle ${EXECUTABLE_TARGET}.app/Contents/MacOS)
           set(_qt_conf_install_dirs ${EXECUTABLE_TARGET}.app/Contents/Resources)
           install(TARGETS ${EXECUTABLE_TARGET} BUNDLE DESTINATION . )
         else()
           if(NOT MACOSX_BUNDLE_NAMES)
             set(_qt_conf_install_dirs bin)
             set(_target_locations bin/${EXECUTABLE_TARGET})
             set(${_target_locations}_qt_plugins_install_dir bin)
             install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
           else()
             foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
               list(APPEND _qt_conf_install_dirs ${bundle_name}.app/Contents/Resources)
               set(_current_target_location ${bundle_name}.app/Contents/MacOS/${EXECUTABLE_TARGET})
               list(APPEND _target_locations ${_current_target_location})
               set(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS)
               message( "  set(${_current_target_location}_qt_plugins_install_dir ${bundle_name}.app/Contents/MacOS) ")

               install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION ${bundle_name}.app/Contents/MacOS/)
             endforeach()
           endif()
         endif()
       else()
         set(_target_locations bin/${EXECUTABLE_TARGET}${CMAKE_EXECUTABLE_SUFFIX})
         set(${_target_locations}_qt_plugins_install_dir bin)
         set(_qt_conf_install_dirs bin)
         install(TARGETS ${EXECUTABLE_TARGET} RUNTIME DESTINATION bin)
       endif()
      endif()
    endforeach()

  # This mini app does not depend on mitkDiffusionImaging at all

  #mitk_create_executable(CLGlobalImageFeatures
  #  DEPENDS MitkCore MitkCLUtilities
  #  CPP_FILES CLGlobalImageFeatures.cpp mitkCommandLineParser.cpp
  #)

  mitk_create_executable(CLSimpleVoxelClassification
    DEPENDS MitkCore MitkCLCore MitkDataCollection MitkCLVigraRandomForest MitkCommandLine
    CPP_FILES CLSimpleVoxelClassification.cpp
  )
  # This mini app does not depend on mitkDiffusionImaging at all
  mitk_create_executable(CLVoxelClassification
    DEPENDS MitkCore MitkCLCore MitkDataCollection MitkCLImportanceWeighting MitkCLVigraRandomForest
    CPP_FILES CLVoxelClassification.cpp
  )
  mitk_create_executable(CLBrainMask
    DEPENDS MitkCore MitkCLUtilities
    CPP_FILES CLBrainMask.cpp mitkCommandLineParser.cpp
  )
  mitk_create_executable(CLImageConverter
    DEPENDS MitkCore
    CPP_FILES CLImageConverter.cpp mitkCommandLineParser.cpp
  )
  mitk_create_executable(CLSurWeighting
    DEPENDS MitkCore MitkCLUtilities MitkDataCollection MitkCLImportanceWeighting
    CPP_FILES CLSurWeighting.cpp mitkCommandLineParser.cpp
  )
  mitk_create_executable(CLImageCropper
    DEPENDS MitkCore MitkCLUtilities MitkAlgorithmsExt
    CPP_FILES CLImageCropper.cpp mitkCommandLineParser.cpp
  )

        # On Linux, create a shell script to start a relocatable application
        if(UNIX AND NOT APPLE)
          install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ${EXECUTABLE_TARGET}.sh)
        endif()

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES ${EXECUTABLE_TARGET})
  endif()

endif()
