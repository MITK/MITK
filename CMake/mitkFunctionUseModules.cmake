function(_mitk_parse_package_args)
  set(packages ${ARGN})
  set(PACKAGE_NAMES )
  foreach(_package ${packages})
    string(REPLACE "|" ";" _package_list ${_package})
    if("${_package_list}" STREQUAL "${_package}")
      list(APPEND PACKAGE_NAMES ${_package})
    else()
      list(GET _package_list 0 _package_name)
      list(GET _package_list 1 _package_components)
      if(NOT _package_name OR NOT _package_components)
        message(SEND_ERROR "PACKAGE argument syntax wrong. ${_package} is not of the form PACKAGE[|COMPONENT1[+COMPONENT2]...]")
      endif()
      list(APPEND PACKAGE_NAMES ${_package_name})
    endif()
  endforeach()

  if(PACKAGE_NAMES)
    set(package_names_normalized )
    list(REMOVE_DUPLICATES PACKAGE_NAMES)
    list(FIND PACKAGE_NAMES Qt4 _has_qt4_dep)
    list(FIND PACKAGE_NAMES Qt5 _has_qt5_dep)
    foreach(_package_name ${PACKAGE_NAMES})
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
    set(PACKAGE_NAMES ${package_names_normalized})
  endif()

  foreach(_package ${packages})
    string(REPLACE "|" ";" _package_list ${_package})
    if(NOT "${_package_list}" STREQUAL "${_package}")
      list(GET _package_list 0 _package_name)
      list(GET _package_list 1 _package_components)
      string(REPLACE "+" ";" _package_components_list "${_package_components}")
      list(APPEND ${_package_name}_REQUIRED_COMPONENTS ${_package_components_list})
   endif()
  endforeach()

  foreach(_package_name ${PACKAGE_NAMES})
    if(${_package_name}_REQUIRED_COMPONENTS)
      list(REMOVE_DUPLICATES ${_package_name}_REQUIRED_COMPONENTS)
    endif()
    set(${_package_name}_REQUIRED_COMPONENTS ${${_package_name}_REQUIRED_COMPONENTS} PARENT_SCOPE)
  endforeach()
  set(PACKAGE_NAMES ${PACKAGE_NAMES} PARENT_SCOPE)
endfunction()


#! This CMake function sets up the necessary include directories,
#! linker dependencies, and compile flags for a given target which
#! depends on a set of MITK modules or packages.
#!
#! A package argument is of the form
#!
#!   PACKAGE[|COMPONENT1[+COMPONENT2]...]
#!
#! where PACKAGE is the package name (e.g. VTK) and components are
#! the names of required package components or libraries.
#!
#! If a dependency is not available, an error is thrown.
function(mitk_use_modules)

  set(_macro_params
      TARGET           # The target name (required)
      MODULES          # MITK modules which the given TARGET uses
      PACKAGES         # MITK packages which the given TARGET uses
     )

  set(_macro_options )

  MACRO_PARSE_ARGUMENTS(USE "${_macro_params}" "${_macro_options}" ${ARGN})

  # Sanity checks
  if(NOT USE_TARGET)
    message(SEND_ERROR "Required TARGET argument missing.")
  elseif(NOT TARGET ${USE_TARGET})
    message(SEND_ERROR "The given TARGET argument ${USE_TARGET} is not a valid target")
  endif()

  set(ALL_INCLUDE_DIRECTORIES)
  set(ALL_LIBRARIES)
  set(ALL_COMPILE_DEFINITIONS)
  set(ALL_META_DEPENDENCIES)

  set(depends ${USE_MODULES})
  set(package_depends ${USE_PACKAGES})

  # Get transitive MODULE and PACKAGE dependencies
  if(depends)
    set(depends_before "not initialized")
    while(NOT "${depends}" STREQUAL "${depends_before}")
      set(depends_before ${depends})
      foreach(dependency ${depends})
        if(NOT ${dependency}_CONFIG_FILE)
          message(SEND_ERROR "Missing module: ${dependency}")
        endif()
        include(${${dependency}_CONFIG_FILE})
        list(APPEND package_depends ${${dependency}_PACKAGE_DEPENDS})
        list(APPEND depends ${${dependency}_DEPENDS})
      endforeach()

      list(REMOVE_DUPLICATES depends)
      list(SORT depends)
    endwhile()

    # Iterate over all module dependencies
    foreach(dependency ${depends})
      if(${dependency}_IS_DEPRECATED AND NOT MODULE_IS_DEPRECATED)
        # Only print the message if the dependent module
        # is not deprecated itself and if it is a first-level dependency.
        list(FIND USE_MODULES ${dependency} _index)
        if(_index GREATER -1)
          message(WARNING "Module ${dependency} is deprecated since ${${dependency}_DEPRECATED_SINCE}")
        endif()
      endif()

      list(APPEND ALL_INCLUDE_DIRECTORIES ${${dependency}_INCLUDE_DIRS})
      list(APPEND ALL_LIBRARIES ${${dependency}_TARGET})
      if(TARGET ${dependency}-autoload)
        list(APPEND ALL_META_DEPENDENCIES ${dependency}-autoload)
      endif()
    endforeach()

    list(APPEND ALL_INCLUDE_DIRECTORIES ${MODULES_CONF_DIRS})
  endif()

  # Parse package dependencies
  if(package_depends)
    _mitk_parse_package_args(${package_depends})

    # Some package config files like MITK_Qt5_Config.cmake rely on a
    # properly set "MODULE_NAME" variable for the current target.
    set(MODULE_NAME ${USE_TARGET})
    # Read all package information
    foreach(_package ${PACKAGE_NAMES})
      set(${_package}_REQUIRED_COMPONENTS_BY_MODULE ${${_package}_REQUIRED_COMPONENTS})
      set(_package_found 0)
      foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
        if((NOT DEFINED MITK_USE_${_package} OR MITK_USE_${_package}) AND EXISTS "${dir}/MITK_${_package}_Config.cmake")
          include("${dir}/MITK_${_package}_Config.cmake")
          set(_package_found 1)
          break()
        endif()
      endforeach()
      if(NOT _package_found)
        message(SEND_ERROR "Missing package: ${_package}")
      endif()
    endforeach()
  endif()

  if(ALL_INCLUDE_DIRECTORIES)
    include_directories(${ALL_INCLUDE_DIRECTORIES})
  endif()

  if(ALL_COMPILE_DEFINITIONS)
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${ALL_COMPILE_DEFINITIONS})
  endif()

  if(ALL_LIBRARIES)
    target_link_libraries(${USE_TARGET} ${ALL_LIBRARIES})
  endif()

  set(ALL_INCLUDE_DIRECTORIES ${ALL_INCLUDE_DIRECTORIES} PARENT_SCOPE)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} PARENT_SCOPE)
  set(ALL_META_DEPENDENCIES ${ALL_META_DEPENDENCIES} PARENT_SCOPE)

endfunction()
