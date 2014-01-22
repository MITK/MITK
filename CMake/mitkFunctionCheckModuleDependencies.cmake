#! Checks if all required modules and packages exist and stores missing
#! dependencies in <missing_deps>.
#!
#! Usage:
#!
#! mitk_check_module_dependencies(
#!   MODULES <module-list>
#!   PACKAGES <package-list>
#!   MISSING_DEPENDENCIES_VAR <missing_deps>
#!   MODULE_DEPENDENCIES_VAR <module_deps>
#!   PACKAGE_DEPENDENCIES_VAR <package_deps>)
#!
function(mitk_check_module_dependencies)

  set(_macro_params
      MODULES                  # MITK modules which the given TARGET uses
      PACKAGES                 # MITK packages which the given TARGET uses
      MISSING_DEPENDENCIES_VAR # variable for storing missing dependencies
      MODULE_DEPENDENCIES_VAR  # variable for storing all module dependencies
      PACKAGE_DEPENDENCIES_VAR # variable for storing all package dependencies
     )

  set(_macro_options )

  MACRO_PARSE_ARGUMENTS(CHECK "${_macro_params}" "${_macro_options}" ${ARGN})

  set(missing_deps )
  set(depends ${CHECK_MODULES})
  set(package_depends ${CHECK_PACKAGES})

  if(depends)
    set(depends_before "not initialized")
    while(NOT "${depends}" STREQUAL "${depends_before}")
      set(depends_before ${depends})
      foreach(dependency ${depends})
        set(_module_found 1)
        if(NOT ${dependency}_CONFIG_FILE)
          set(_module_found 0)
        endif()
        set(_dependency_file_name ${${dependency}_CONFIG_FILE})
        if(NOT EXISTS ${_dependency_file_name})
          set(_module_found 0)
        endif()

        if(_module_found)
          include(${_dependency_file_name})
          if(${dependency}_IS_ENABLED)
            list(APPEND depends ${${dependency}_DEPENDS})
            list(APPEND package_depends ${${dependency}_PACKAGE_DEPENDS})
          else(${dependency}_IS_ENABLED)
            list(APPEND missing_deps ${dependency})
          endif()
        else()
          list(APPEND missing_deps ${dependency})
        endif()
      endforeach()
      list(REMOVE_DUPLICATES depends)
      list(SORT depends)
    endwhile()
  endif()

  set(package_names_depends)
  if(package_depends)
    list(REMOVE_DUPLICATES package_depends)
    _mitk_parse_package_args(${package_depends})
    foreach(_package ${PACKAGE_NAMES})
      if((DEFINED MITK_USE_${_package}) AND NOT (${MITK_USE_${_package}}))
        list(APPEND missing_deps ${_package})
      endif()
      list(APPEND package_names_depends ${_package})
    endforeach()
  endif()

  if(missing_deps)
    list(REMOVE_DUPLICATES missing_deps)
    if(CHECK_MISSING_DEPENDENCIES_VAR)
      set(${CHECK_MISSING_DEPENDENCIES_VAR} ${missing_deps} PARENT_SCOPE)
    endif()
  endif()
  if(CHECK_MODULE_DEPENDENCIES_VAR)
    set(${CHECK_MODULE_DEPENDENCIES_VAR} ${depends} PARENT_SCOPE)
  endif()
  if(CHECK_PACKAGE_DEPENDENCIES_VAR)
    if(package_depends)
      list(REMOVE_DUPLICATES package_names_depends)
    endif()
    set(${CHECK_PACKAGE_DEPENDENCIES_VAR} ${package_names_depends} PARENT_SCOPE)
  endif()
endfunction()
