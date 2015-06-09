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
      MISSING_DEPENDENCIES_VAR # variable for storing missing dependencies
      MODULE_DEPENDENCIES_VAR  # variable for storing all module dependencies
      PACKAGE_DEPENDENCIES_VAR # variable for storing all package dependencies
     )

  set(_macro_multiparams
      MODULES                  # MITK modules which the given TARGET uses
      PACKAGES                 # MITK packages which the given TARGET uses
     )

  set(_macro_options )

  cmake_parse_arguments(CHECK "" "${_macro_params}" "${_macro_multiparams}" ${ARGN})

  set(missing_deps )
  set(depends ${CHECK_MODULES})
  set(package_depends ${CHECK_PACKAGES})
  set(module_names )
  set(package_names )

  foreach(dep ${depends})
    if(NOT dep STREQUAL "PUBLIC" AND NOT dep STREQUAL "PRIVATE" AND NOT dep STREQUAL "INTERFACE")
      if(NOT TARGET ${dep})
        list(APPEND missing_deps ${dep})
      endif()
      list(APPEND module_names ${dep})
    endif()
  endforeach()

  set(package_names)
  if(package_depends)
    _mitk_parse_package_args(${package_depends})
    set(package_names ${PUBLIC_PACKAGE_NAMES} ${PRIVATE_PACKAGE_NAMES} ${INTERFACE_PACKAGE_NAMES})
    list(REMOVE_DUPLICATES package_names)
    foreach(_package ${package_names})
      if((DEFINED MITK_USE_${_package}) AND NOT (${MITK_USE_${_package}}))
        list(APPEND missing_deps ${_package})
      endif()
    endforeach()
  endif()

  if(missing_deps)
    list(REMOVE_DUPLICATES missing_deps)
    if(CHECK_MISSING_DEPENDENCIES_VAR)
      set(${CHECK_MISSING_DEPENDENCIES_VAR} ${missing_deps} PARENT_SCOPE)
    endif()
  endif()
  if(CHECK_MODULE_DEPENDENCIES_VAR)
    set(${CHECK_MODULE_DEPENDENCIES_VAR} ${module_names} PARENT_SCOPE)
  endif()
  if(CHECK_PACKAGE_DEPENDENCIES_VAR)
    set(${CHECK_PACKAGE_DEPENDENCIES_VAR} ${package_names} PARENT_SCOPE)
  endif()
endfunction()
