##################################################################
#
# MITK_CREATE_EXECUTABLE
#
#! Creates an executable with MITK dependencies and batch files
#! for proper application start-up.
#!
#! USAGE:
#!
#! \code
#! MITK_CREATE_EXECUTABLE( [<exectuableName>]
#!     [DEPENDS <modules we need>]
#!     [PACKAGE_DEPENDS <packages we need, like ITK, VTK, QT>]
#!     [INCLUDE_DIRS <list of additional include directories>]
#!     [TARGET_DEPENDS <list of additional dependencies>
#!     [WARNINGS_NO_ERRORS]
#!     [NO_INSTALL]
#! \endcode
#!
#! \param EXECUTABLE_NAME The name for the new executable target
##################################################################
macro(mitk_create_executable)

  set(_macro_params
      VERSION                # version number, e.g. "1.2.0"
      FILES_CMAKE            # file name of a CMake file setting source list variables
                             # (defaults to files.cmake)
      DESCRIPTION            # a description for the executable
     )

  set(_macro_multiparams
      SUBPROJECTS            # list of CDash labels (deprecated)
      INCLUDE_DIRS           # additional include dirs
      DEPENDS                # list of modules this module depends on
      PACKAGE_DEPENDS        # list of "packages" this module depends on (e.g. Qt, VTK, etc.)
      TARGET_DEPENDS         # list of CMake targets this executable should depend on
      ADDITIONAL_LIBS        # list of additional libraries linked to this executable
      CPP_FILES              # (optional) list of cpp files
     )

  set(_macro_options
      NO_INIT                # do not create CppMicroServices initialization code
      NO_FEATURE_INFO        # do not create a feature info by calling add_feature_info()
      NO_BATCH_FILE          # do not create batch files on Windows
      WARNINGS_NO_ERRORS     # do not treat compiler warnings as errors
      NO_INSTALL
     )

  cmake_parse_arguments(EXEC "${_macro_options}" "${_macro_params}" "${_macro_multiparams}" ${ARGN})

  set(_EXEC_OPTIONS EXECUTABLE)
  if(EXEC_NO_INIT)
    list(APPEND _EXEC_OPTIONS NO_INIT)
  endif()
  if(EXEC_WARNINGS_NO_ERRORS)
    list(APPEND _EXEC_OPTIONS WARNINGS_NO_ERRORS)
  endif()
  if(EXEC_NO_FEATURE_INFO)
    list(APPEND _EXEC_OPTIONS NO_FEATURE_INFO)
  endif()

  mitk_create_module(${EXEC_UNPARSED_ARGUMENTS}
                     VERSION ${EXEC_VERSION}
                     INCLUDE_DIRS ${EXEC_INCLUDE_DIRS}
                     DEPENDS ${EXEC_DEPENDS}
                     PACKAGE_DEPENDS ${EXEC_PACKAGE_DEPENDS}
                     TARGET_DEPENDS ${EXEC_TARGET_DEPENDS}
                     ADDITIONAL_LIBS ${EXEC_ADDITIONAL_LIBS}
                     FILES_CMAKE ${EXEC_FILES_CMAKE}
                     CPP_FILES ${EXEC_CPP_FILES}
                     DESCRIPTION "${DESCRIPTION}"
                     ${_EXEC_OPTIONS}
                    )

  set(EXECUTABLE_IS_ENABLED ${MODULE_IS_ENABLED})
  set(EXECUTABLE_TARGET ${MODULE_TARGET})
  if(EXECUTABLE_IS_ENABLED)
    set_property(GLOBAL APPEND PROPERTY MITK_EXECUTABLE_TARGETS ${EXECUTABLE_TARGET})
    if(EXEC_NO_INSTALL)
      set_target_properties(${EXECUTABLE_TARGET} PROPERTIES NO_INSTALL TRUE)
    endif()
    # Add meta dependencies (e.g. on auto-load modules from depending modules)
    if(TARGET ${CMAKE_PROJECT_NAME}-autoload)
      add_dependencies(${MODULE_TARGET} ${CMAKE_PROJECT_NAME}-autoload)
    endif()

    # Create batch and VS user files for Windows platforms
    include(mitkFunctionCreateWindowsBatchScript)
    if(WIN32)
      set(_batch_file_in "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_TARGET}.bat.in")
      if(NOT EXISTS "${_batch_file_in}")
        set(_batch_file_in "${MITK_CMAKE_DIR}/StartApp.bat.in")
      endif()
      if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(_batch_file_out_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
      else()
        set(_batch_file_out_dir "${CMAKE_CURRENT_BINARY_DIR}")
      endif()
      if(NOT EXEC_NO_BATCH_FILE)
        if(NOT EXEC_NAME)
          set(EXEC_NAME ${MODULE_TARGET})
        endif()
        foreach(BUILD_TYPE debug release)
          mitkFunctionCreateWindowsBatchScript(
              ${_batch_file_in} ${_batch_file_out_dir}/${MODULE_TARGET}_${BUILD_TYPE}.bat
              ${BUILD_TYPE}
             )
        endforeach()
      endif()
      mitkFunctionConfigureVisualStudioUserProjectFile(
          NAME ${MODULE_TARGET}
        )
    endif()
  endif()

endmacro()
