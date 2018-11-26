option(BUILD_BasicImageProcessingMiniApps "Build commandline tools for Basic Image Processing" OFF)

if(BUILD_BasicImageProcessingMiniApps OR MITK_BUILD_ALL_APPS)


  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    # list of miniapps
    # if an app requires additional dependencies
    # they are added after a "^^" and separated by "_"
    set( basicImageProcessingMiniApps
        FileConverter^^MitkCore
        ImageTypeConverter^^MitkCore
        SingleImageArithmetic^^MitkCore_MitkBasicImageProcessing
        TwoImageArithmetic^^MitkCore_MitkBasicImageProcessing
        ImageAndValueArithmetic^^MitkCore_MitkBasicImageProcessing
        MaskRangeBasedFiltering^^MitkCore_MitkBasicImageProcessing
        MaskOutlierFiltering^^MitkCore_MitkBasicImageProcessing
        ResampleImage^^MitkCore_MitkBasicImageProcessing
        ResampleMask^^MitkCore_MitkBasicImageProcessing
        LaplacianOfGaussian^^MitkCore_MitkBasicImageProcessing
        MultiResolutionPyramid^^MitkCore_MitkBasicImageProcessing
        ForwardWavelet^^MitkCore_MitkBasicImageProcessing
    )

    foreach(basicImageProcessingMiniApp ${basicImageProcessingMiniApps})
      # extract mini app name and dependencies
      string(REPLACE "^^" "\\;" miniapp_info ${basicImageProcessingMiniApp})
      set(miniapp_info_list ${miniapp_info})
      list(GET miniapp_info_list 0 appname)
      list(GET miniapp_info_list 1 raw_dependencies)
      string(REPLACE "_" "\\;" dependencies "${raw_dependencies}")
      set(dependencies_list ${dependencies})

      mitk_create_executable(${appname}
      DEPENDS MitkCore MitkCommandLine ${dependencies_list}
      PACKAGE_DEPENDS ITK
      CPP_FILES ${appname}.cpp
      )
      # CPP_FILES ${appname}.cpp mitkCommandLineParser.cpp

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

    # On Linux, create a shell script to start a relocatable application
    if(UNIX AND NOT APPLE)
      install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ${EXECUTABLE_TARGET}.sh)
    endif()

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES ${EXECUTABLE_TARGET})
  endif()

endif()
