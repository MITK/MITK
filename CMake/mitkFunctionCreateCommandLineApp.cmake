#!
#! Create a Command Line App.
#!
#! \brief This function will create a command line executable and the scripts required to run it
#!
#! \param NAME (required) Name of the command line app
#! \param DEPENDS (optional) Required MITK modules beyond MitkCommandLine
#! \param PACKAGE_DEPENDS (optional) list of "packages" this command line app depends on (e.g. ITK, VTK, etc.)
#! \param CPP_FILES (optional) list of cpp files, if it is not given NAME.cpp is assumed
#!
#! Assuming that there exists a file called <code>MyApp.cpp</code>, an example call looks like:
#! \code
#! mitkFunctionCreateCommandLineApp(
#!   NAME MyApp
#!   DEPENDS MitkCore MitkPlanarFigure
#!   PACKAGE_DEPENDS ITK VTK
#! )
#! \endcode
#!

function(mitkFunctionCreateCommandLineApp)

  set(_function_params
      NAME                # Name of the command line app
     )

  set(_function_multiparams
      DEPENDS                # list of modules this command line app depends on
      PACKAGE_DEPENDS        # list of "packages" this command line app depends on (e.g. ITK, VTK, etc.)
      CPP_FILES              # (optional) list of cpp files, if it is not given NAME.cpp is assumed
     )

  set(_function_options

     )

  cmake_parse_arguments(CMDAPP "${_function_options}" "${_function_params}" "${_function_multiparams}" ${ARGN})

  if(NOT CMDAPP_NAME)
    message(FATAL_ERROR "NAME argument cannot be empty.")
  endif()

  if(NOT CMDAPP_CPP_FILES)
    set(CMDAPP_CPP_FILES ${CMDAPP_NAME}.cpp)
  endif()

  mitk_create_executable(${CMDAPP_NAME}
  DEPENDS MitkCommandLine ${CMDAPP_DEPENDS}
  PACKAGE_DEPENDS ${CMDAPP_PACKAGE_DEPENDS}
  CPP_FILES ${CMDAPP_CPP_FILES}
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
endfunction()
