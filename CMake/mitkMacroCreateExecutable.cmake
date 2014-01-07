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
#! MITK_CREATE_EXECUTABLE( <exectuableName>
#!     [DEPENDS <modules we need>]
#!     [PACKAGE_DEPENDS <packages we need, like ITK, VTK, QT>]
#!     [INCLUDE_DIRS <list of additional include directories>]
#!     [TARGET_DEPENDS <list of additional dependencies>
#!     [WARNINGS_AS_ERRORS]
#! \endcode
#!
#! \param EXECUTABLE_NAME The name for the new executable target
##################################################################
macro(mitk_create_executable EXECUTABLE_NAME)

  set(_macro_params
      SUBPROJECTS            # list of CDash labels
      VERSION                # version number, e.g. "1.2.0"
      INCLUDE_DIRS           # additional include dirs
      DEPENDS                # list of modules this module depends on
      PACKAGE_DEPENDS        # list of "packages" this module depends on (e.g. Qt, VTK, etc.)
      TARGET_DEPENDS         # list of CMake targets this executable should depend on
      ADDITIONAL_LIBS        # list of additional libraries linked to this executable
      QT4_MODULES            # the executable depends on a given list of Qt 4 modules
      QT5_MODULES            # the executable depends on a given list of Qt 5 modules
     )

  set(_macro_options
      NO_INIT                # do not create CppMicroServices initialization code
      NO_BATCH_FILE          # do not create batch files on Windows
      WARNINGS_AS_ERRORS     # treat all compiler warnings as errors
     )

  MACRO_PARSE_ARGUMENTS(EXEC "${_macro_params}" "${_macro_options}" ${ARGN})

  set(EXEC_NAME ${EXECUTABLE_NAME})

  set(_EXEC_OPTIONS EXECUTABLE)
  if(EXEC_NO_INIT)
    list(APPEND _EXEC_OPTIONS NO_INIT)
  endif()
  if(EXEC_WARNINGS_AS_ERRORS)
    list(APPEND _EXEC_OPTIONS WARNINGS_AS_ERRORS)
  endif()

  mitk_create_module(${EXEC_NAME}
                     SUBPROJECTS ${EXEC_SUBPROJECTS}
                     VERSION ${EXEC_VERSION}
                     INCLUDE_DIRS ${EXEC_INCLUDE_DIRS}
                     DEPENDS ${EXEC_DEPENDS}
                     PACKAGE_DEPENDS ${EXEC_PACKAGE_DEPENDS}
                     TARGET_DEPENDS ${EXEC_TARGET_DEPENDS}
                     ADDITIONAL_LIBS ${EXEC_ADDITIONAL_LIBS}
                     QT4_MODULES ${EXEC_QT4_MODULES}
                     QT5_MODULES ${EXEC_QT5_MODULES}
                     ${_EXEC_OPTIONS}
                    )

  # Create batch files for Windows platforms
  if(WIN32)
    set(_batch_file_in "${CMAKE_CURRENT_SOURCE_DIR}/${EXEC_NAME}.bat.in")
    if(NOT EXISTS "${_batch_file_in}")
      set(_batch_file_in "${MITK_CMAKE_DIR}/StartApp.bat.in")
    endif()
    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      set(_batch_file_out_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
      set(_batch_file_out_dir "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    if(NOT EXEC_NO_BATCH_FILE)
      foreach(BUILD_TYPE debug release)
        mitkFunctionCreateWindowsBatchScript(
            ${_batch_file_in} ${_batch_file_out_dir}/${EXEC_NAME}_${BUILD_TYPE}.bat
            ${BUILD_TYPE}
           )
      endforeach()
    endif()
  endif()

endmacro()
