#! Register an external project.
#!
#! Calling this function registers an external project with the MITK
#! build system. Usage:
#!
#!   mitkFunctionAddExternalProject(NAME name [ON|OFF]
#!     [PACKAGE package] [COMPONENTS comp1...]
#!     [DEPENDS dep1...]
#!     [DOC docstring]
#!     [NO_CACHE] [ADVANCED] [NO_PACKAGE]
#!   )
#!
#! The function creates a MITK_USE_<name> internal cache variable if the
#! NO_CACHE option is given. If the NO_CACHE option is not set, a normal
#! boolean cache option is created with the given DOC argument as help string.
#! The option is marked as advanced if the ADVANCED option is set. The
#! option's initial value matches the ON or OFF option and defaults to OFF
#! if none is given. The DEPENDS arguments are used to force the
#! corresponding MITK_USE_<depN> cache variables to ON if MITK_USE_<name>
#! is ON.
#!
#! The PACKAGE argument names the argument for the
#! corresponding find_package() call in MITKs top-level CMakeLists.txt
#! and MITKConfig.cmake file. It defaults to <name>. If COMPONENTS are
#! specified, these are passed to the find_package() call. If the
#! NO_PACKAGE option is given, the find_package() call is suppressed.
#!
#! For each registered external project there must exist a file called
#! CMakeExternals/<name>.cmake which defines the build process for
#! the external project.
#!
#! \note Note that multiple calls of this function must be ordered
#! relative to their declared dependencies. This applies to the DEPENDS
#! arguments of this function as well as the actual target dependencies
#! declared in the CMakeExternals/<name>.cmake file.
#!
function(mitkFunctionAddExternalProject)
  cmake_parse_arguments(EP "ON;OFF;NO_CACHE;ADVANCED;NO_PACKAGE" "NAME;DOC;PACKAGE" "DEPENDS;COMPONENTS" ${ARGN})
  if(NOT EP_NAME)
    message(SEND_ERROR "The NAME argument is missing.")
  endif()
  set_property(GLOBAL APPEND PROPERTY MITK_EXTERNAL_PROJECTS ${EP_NAME})
  if(NOT EP_DOC)
    set(EP_DOC "Use ${EP_NAME}")
  endif()
  if(NOT EP_PACKAGE AND NOT EP_NO_PACKAGE)
    set(EP_PACKAGE ${EP_NAME})
  endif()

  set(_use_var "MITK_USE_${EP_NAME}")
  set(_on 0)
  if(DEFINED ${_use_var})
    set(_on ${${_use_var}})
  else()
    set(_on ${EP_ON})
  endif()

  if(_on AND EP_DEPENDS)
    # Get all transitive dependencies
    set(_depends_all)
    set(_depends_cur ${EP_DEPENDS})
    list(REMOVE_DUPLICATES _depends_cur)
    set(_depends_new ${_depends_cur})
    while(NOT "${_depends_all}" STREQUAL "${_depends_cur}")
      list(APPEND _depends_all ${_depends_new})
      set(_depends_new_tmp )
      foreach(dep ${_depends_new})
        get_property(_dep_dep GLOBAL PROPERTY MITK_${dep}_DEPENDS)
        list(APPEND _depends_new_tmp ${_dep_dep})
      endforeach()
      set(_depends_new ${_depends_new_tmp})
      if(_depends_new)
        list(REMOVE_DUPLICATES _depends_new)
        list(APPEND _depends_cur ${_depends_new})
      endif()
    endwhile()
    # Force dependencies to ON
    foreach(dep ${_depends_all})
      if(NOT MITK_USE_${dep})
        get_property(_depends_doc CACHE MITK_USE_${dep} PROPERTY HELPSTRING)
        get_property(_depends_type CACHE MITK_USE_${dep} PROPERTY TYPE)
        if(NOT TYPE STREQUAL "INTERNAL")
          # This is a cache UI variable
          message("> Forcing MITK_USE_${dep} to ON because of ${_use_var}")
          set(MITK_USE_${dep} ON CACHE BOOL "${_depends_doc}" FORCE)
        else()
          # This is an internal cache variable
          set(MITK_USE_${dep} ON CACHE INTERNAL "${_depends_doc}" FORCE)
        endif()
      endif()
    endforeach()
  endif()

  # Set the actual MITK_USE_<name> cache variable
  if(EP_NO_CACHE)
    set(${_use_var} ${_on} CACHE INTERNAL "${EP_DOC}" FORCE)
  else()
    option(${_use_var} "${EP_DOC}" ${_on})
    #set(${_use_var} ${_on} CACHE BOOL "${EP_DOC}" FORCE)
    if(EP_ADVANCED)
      mark_as_advanced(${_use_var})
    endif()
  endif()
  set_property(GLOBAL PROPERTY MITK_${EP_NAME}_PACKAGE ${EP_PACKAGE})
  set_property(GLOBAL PROPERTY MITK_${EP_NAME}_COMPONENTS "${EP_COMPONENTS}")
  set_property(GLOBAL PROPERTY MITK_${EP_NAME}_DEPENDS "${EP_DEPENDS}")
endfunction()
