macro(_mitk_normalize_package_names _package_names_var)
  set(_package_names_var ${_package_names_var})
  if(${_package_names_var})
    set(package_names_normalized )
    list(REMOVE_DUPLICATES ${_package_names_var})
    if(NOT DEFINED _has_qt4_dep OR _has_qt4_dep EQUAL -1)
      list(FIND ${_package_names_var} Qt4 _has_qt4_dep)
    endif()
    if(NOT DEFINED _has_qt5_dep OR _has_qt5_dep EQUAL -1)
      list(FIND ${_package_names_var} Qt5 _has_qt5_dep)
    endif()
    foreach(_package_name ${${_package_names_var}})
      set(${_package_name}_REQUIRED_COMPONENTS )
      # Special filter for exclusive OR Qt4 / Qt5 dependency
      if(_package_name STREQUAL "Qt4")
        if(MITK_USE_Qt4 OR NOT MITK_USE_Qt5)
          # MITK_USE_Qt4 is ON or both MITK_USE_Qt4 and MITK_USE_Qt5
          # are OFF. So list Qt4 as a dependency.
          list(APPEND package_names_normalized ${_package_name})
        elseif(MITK_USE_Qt5 AND _has_qt5_dep EQUAL -1)
            # List Qt4 as a dependency only if there is no Qt5 dependency
            # so the module will not be build because of the missing
            # MITK_USE_Qt4
            list(APPEND package_names_normalized ${_package_name})
        endif()
      elseif(_package_name STREQUAL "Qt5")
        if(MITK_USE_Qt5 OR NOT MITK_USE_Qt4)
          list(APPEND package_names_normalized ${_package_name})
        elseif(MITK_USE_Qt4 AND _has_qt4_dep EQUAL -1)
          list(APPEND package_names_normalized ${_package_name})
        endif()
      else()
        list(APPEND package_names_normalized ${_package_name})
      endif()
    endforeach()
    set(${_package_names_var} ${package_names_normalized})
  endif()
endmacro()

function(_mitk_parse_package_args)
  set(package_list ${ARGN})

  set(PUBLIC_PACKAGE_NAMES )
  set(PRIVATE_PACKAGE_NAMES )
  set(INTERFACE_PACKAGE_NAMES )

  set(_package_visibility PRIVATE)
  foreach(_package ${package_list})
    if(_package STREQUAL "PUBLIC" OR _package STREQUAL "PRIVATE" OR _package STREQUAL "INTERFACE")
      set(_package_visibility ${_package})
    else()
      list(APPEND packages ${_package})
      set(_package_name )
      set(_package_components_list )
      string(REPLACE "|" ";" _package_list ${_package})
      if("${_package_list}" STREQUAL "${_package}")
        set(_package_name ${_package})
      else()
        list(GET _package_list 0 _package_name)
        list(GET _package_list 1 _package_components)
        string(REPLACE "+" ";" _package_components_list "${_package_components}")
        if(NOT _package_name OR NOT _package_components)
          message(SEND_ERROR "PACKAGE argument syntax wrong. ${_package} is not of the form PACKAGE[|COMPONENT1[+COMPONENT2]...]")
        endif()
      endif()
      list(APPEND ${_package_visibility}_PACKAGE_NAMES ${_package_name})
      list(APPEND ${_package_visibility}_${_package_name}_REQUIRED_COMPONENTS ${_package_components_list})
    endif()
  endforeach()

  # Check if both Qt4 and Qt5 packages are specified in one of the
  # public/private/interface lists
  set(_package_names ${PUBLIC_PACKAGE_NAMES} ${PRIVATE_PACKAGE_NAMES} ${INTERFACE_PACKAGE_NAMES})
  list(FIND _package_names Qt4 _has_qt4_dep)
  list(FIND _package_names Qt5 _has_qt5_dep)

  _mitk_normalize_package_names(PUBLIC_PACKAGE_NAMES)
  _mitk_normalize_package_names(PRIVATE_PACKAGE_NAMES)
  _mitk_normalize_package_names(INTERFACE_PACKAGE_NAMES)

  # remove duplicates and set package components in parent scope
  foreach(_package_visibility PUBLIC PRIVATE INTERFACE)
    foreach(_package_name ${${_package_visibility}_PACKAGE_NAMES})
      if(${_package_visibility}_${_package_name}_REQUIRED_COMPONENTS)
        list(REMOVE_DUPLICATES ${_package_visibility}_${_package_name}_REQUIRED_COMPONENTS)
      endif()
      set(${_package_visibility}_${_package_name}_REQUIRED_COMPONENTS ${${_package_visibility}_${_package_name}_REQUIRED_COMPONENTS} PARENT_SCOPE)
    endforeach()
  endforeach()

  set(PUBLIC_PACKAGE_NAMES ${PUBLIC_PACKAGE_NAMES} PARENT_SCOPE)
  set(PRIVATE_PACKAGE_NAMES ${PRIVATE_PACKAGE_NAMES} PARENT_SCOPE)
  set(INTERFACE_PACKAGE_NAMES ${INTERFACE_PACKAGE_NAMES} PARENT_SCOPE)
  set(PACKAGE_NAMES ${PUBLIC_PACKAGE_NAMES} ${PRIVATE_PACKAGE_NAMES} ${INTERFACE_PACKAGE_NAMES} PARENT_SCOPE)
endfunction()

function(_include_package_config pkg_config_file)
  # wrap the inclusion of the MITK_<pkg>_Config.cmake file in a
  # function to create a scope for its variables; this allows
  # multiple inclusions of the file in the parent scope
  include(${pkg_config_file})
  set(ALL_INCLUDE_DIRECTORIES ${ALL_INCLUDE_DIRECTORIES} PARENT_SCOPE)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} PARENT_SCOPE)
  set(ALL_COMPILE_DEFINITIONS ${ALL_COMPILE_DEFINITIONS} PARENT_SCOPE)
  set(ALL_COMPILE_OPTIONS ${ALL_COMPILE_OPTIONS} PARENT_SCOPE)
endfunction()

#! This CMake function sets up the necessary include directories,
#! linker dependencies, and compile flags for a given target which
#! depends on a set of MITK modules or packages.
#!
#! A package argument is of the form
#!
#!   [PUBLIC|PRIVATE|INTERFACE] PACKAGE[|COMPONENT1[+COMPONENT2]...]
#!
#! where PACKAGE is the package name (e.g. VTK) and components are
#! the names of required package components or libraries.
#!
#! If a dependency is not available, an error is thrown.
function(mitk_use_modules)

  set(_macro_params
      TARGET           # The target name (required)
     )

  set(_macro_multiparams
      MODULES          # MITK modules which the given TARGET uses
      PACKAGES         # MITK packages which the given TARGET uses
     )

  set(_macro_options )

  cmake_parse_arguments(USE "${_macro_options}" "${_macro_params}" "${_macro_multiparams}" ${ARGN})

  # Sanity checks
  if(NOT USE_TARGET)
    message(SEND_ERROR "Required TARGET argument missing.")
  elseif(NOT TARGET ${USE_TARGET})
    message(SEND_ERROR "The given TARGET argument ${USE_TARGET} is not a valid target")
  endif()

  set(depends ${USE_MODULES})
  set(package_depends ${USE_PACKAGES})

  if(depends)
    # Iterate over all module dependencies
    foreach(dependency ${depends})
      if(TARGET ${dependency} AND NOT MODULE_IS_DEPRECATED)
        get_target_property(_is_interface_lib ${dependency} TYPE)
        if(NOT _is_interface_lib)
          get_target_property(_dependency_deprecated_since ${dependency} MITK_MODULE_DEPRECATED_SINCE)
          if(_dependency_deprecated_since)
            message(WARNING "Module ${dependency} is deprecated since ${_dependency_deprecated_since}")
          endif()
        endif()
      endif()
    endforeach()
    target_link_libraries(${USE_TARGET} PUBLIC ${depends})
  endif()

  # Parse package dependencies
  if(package_depends)
    _mitk_parse_package_args(${package_depends})

    # Some package config files like MITK_Qt5_Config.cmake rely on a
    # properly set "MODULE_NAME" variable for the current target.
    set(MODULE_NAME ${USE_TARGET})
    # Read all package information
    foreach(_package_visibility INTERFACE PUBLIC PRIVATE)
    foreach(_package ${${_package_visibility}_PACKAGE_NAMES})
      set(ALL_INCLUDE_DIRECTORIES)
      set(ALL_LIBRARIES)
      set(ALL_COMPILE_DEFINITIONS)
      set(ALL_COMPILE_OPTIONS)

      set(${_package}_REQUIRED_COMPONENTS_BY_MODULE ${${_package_visibility}_${_package}_REQUIRED_COMPONENTS})
      set(_package_found 0)
      foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
        if((NOT DEFINED MITK_USE_${_package} OR MITK_USE_${_package}) AND EXISTS "${dir}/MITK_${_package}_Config.cmake")
          _include_package_config("${dir}/MITK_${_package}_Config.cmake")
          set(_package_found 1)
          break()
        endif()
      endforeach()
      if(_package_found)
        if(ALL_INCLUDE_DIRECTORIES)
          list(REMOVE_DUPLICATES ALL_INCLUDE_DIRECTORIES)
          target_include_directories(${USE_TARGET} SYSTEM ${_package_visibility} ${ALL_INCLUDE_DIRECTORIES})
        endif()
        if(ALL_LIBRARIES)
          # Don't remove "duplicats" because ALL_LIBRARIES may be of the form:
          # "general;bla;debug;blad;general;foo;debug;food"
          target_link_libraries(${USE_TARGET} ${_package_visibility} ${ALL_LIBRARIES})
        endif()
        if(ALL_COMPILE_DEFINITIONS)
          list(REMOVE_DUPLICATES ALL_COMPILE_DEFINITIONS)
          # Compile definitions are always added "PRIVATE" to avoid multiple definitions
          # on the command line due to transitive and direct dependencies adding the
          # same definitions.
          target_compile_definitions(${USE_TARGET} PRIVATE ${ALL_COMPILE_DEFINITIONS})
        endif()
        if(ALL_COMPILE_OPTIONS)
          list(REMOVE_DUPLICATES ALL_COMPILE_OPTIONS)
          target_compile_options(${USE_TARGET} ${_package_visibility} ${ALL_COMPILE_OPTIONS})
        endif()
      else()
        message(SEND_ERROR "Missing package: ${_package}")
      endif()
    endforeach()
    endforeach()
  endif()
endfunction()
