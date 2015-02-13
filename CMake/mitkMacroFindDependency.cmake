#! Wraps a find_package call by forwarding QUIET and REQUIRED arguments
#! from within the MITKConfig.cmake file.
#!
#!   mitkMacroFindDependency(<dep> [args])
#!
macro(mitkMacroFindDependency dep)

  set(_mitk_fd_dep ${dep})
  if(NOT _mitk_fd_dep)
    message(FATAL_ERROR "Missing argument.")
  endif()

  string(TOUPPER "${_mitk_fd_dep}" _mitk_fd_dep_uc)

  if(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    set(_mitk_fd_quiet_arg QUIET)
  endif()
  set(_mitk_fd_required_arg)
  if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
    set(_mitk_fd_required_arg REQUIRED)
  endif()

  get_property(_mitk_fd_is_transitive GLOBAL PROPERTY
    _CMAKE_${_mitk_fd_dep}_TRANSITIVE_DEPENDENCY
  )

  # Prefer config mode first because it finds external
  # <proj>Config.cmake files pointed at by <proj>_DIR variables.
  # Otherwise, existing Find<proj>.cmake files could fail.
  # (e.g. in the case of GLEW and the FindGLEW.cmake file shipped
  # with CMake).
  find_package(${_mitk_fd_dep} ${ARGN}
    QUIET
    CONFIG
  )
  if(NOT (${_mitk_fd_dep}_FOUND OR ${_mitk_fd_dep_uc}_FOUND))
    find_package(${_mitk_fd_dep} ${ARGN}
      ${cmake_fd_quiet_arg}
      ${cmake_fd_required_arg}
    )
  endif()

  if(NOT DEFINED _mitk_fd_is_transitive OR _mitk_fd_is_transitive)
    set_property(GLOBAL PROPERTY _CMAKE_${_mitk_fd_dep}_TRANSITIVE_DEPENDENCY TRUE)
  endif()

  if (NOT (${_mitk_fd_dep}_FOUND OR ${_mitk_fd_dep_uc}_FOUND))
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE "${CMAKE_FIND_PACKAGE_NAME} could not be found because dependency ${_mitk_fd_dep} could not be found.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND False)
    return()
  endif()
  set(_mitk_fd_required_arg)
  set(_mitk_fd_quiet_arg)

endmacro()
