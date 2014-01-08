#! This CMake function sets up the necessary include directories,
#! linker dependencies, and compile flags for a given target which
#! depends on a set of MITK modules, packages, and Qt4 and/or Qt5
#! components.
#! If a dependency is not available, an error is thrown.
function(mitk_use_modules)

  set(_macro_params
      TARGET           # The target name (required)
      MODULES          # MITK modules which the given TARGET uses
      PACKAGES         # MITK packages which the given TARGET uses
      QT4_MODULES      # Qt4 components the TARGET depends on
      QT5_MODULES      # Qt5 components the TARGET depends on
     )

  set(_macro_options )

  MACRO_PARSE_ARGUMENTS(USE "${_macro_params}" "${_macro_options}" ${ARGN})

  # Sanity checks
  if(NOT USE_TARGET)
    message(SEND_ERROR "Required TARGET argument missing.")
  elseif(NOT TARGET ${USE_TARGET})
    message(SEND_ERROR "The given TARGET argument ${USE_TARGET} is not a valid target")
  endif()
  set(all_deps ${USE_MODULES} ${USE_PACKAGES})
  set(all_args MODULES PACKAGES)
  if(MITK_USE_Qt4)
    list(APPEND all_deps ${USE_QT4_MODULES})
    list(APPEND all_args QT4_MODULES)
  elseif(MITK_USE_Qt5)
    list(APPEND all_deps ${USE_QT5_MODULES})
    list(APPEND all_args QT5_MODULES)
  endif()
  if(NOT all_deps)
    message(SEND_ERROR "The arguments ${all_args} must not all be empty.")
  endif()

  # The MODULE_NAME variable is used for example in the MITK_Qt5*_Config.cmake files
  set(MODULE_NAME ${USE_TARGET})

  set(depends "")
  # check for each parameter if it is a package (3rd party)
  set(package_candidates ${USE_MODULES} ${USE_PACKAGES})
  if(MITK_USE_Qt4 AND USE_QT4_MODULES)
    list(APPEND package_candidates Qt4)
  endif()
  if(MITK_USE_Qt5 AND USE_QT5_MODULES)
    list(APPEND package_candidates ${USE_QT5_MODULES})
  endif()
  set(package_depends )
  if(package_candidates)
    foreach(package ${package_candidates})
      set(is_package)
      foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
        if(EXISTS "${dir}/MITK_${package}_Config.cmake")
          list(APPEND package_depends ${package})
          set(is_package 1)
          break()
        endif()
      endforeach()
      if(NOT is_package)
        list(APPEND depends ${package})
      endif()
    endforeach()
  endif()

  set(first_level_module_deps ${depends})
  set(depends_before "not initialized")
  while(NOT "${depends}" STREQUAL "${depends_before}")
    set(depends_before ${depends})
    foreach(dependency ${depends})
      if(NOT ${dependency}_CONFIG_FILE)
        message(SEND_ERROR "Missing module: ${dependency}")
      endif()
      include(${${dependency}_CONFIG_FILE})
      list(APPEND depends ${${dependency}_DEPENDS})
      list(APPEND package_depends ${${dependency}_PACKAGE_DEPENDS})
    endforeach()

    if(depends)
      list(REMOVE_DUPLICATES depends)
      list(SORT depends)
    endif()

    if(package_depends)
      list(REMOVE_DUPLICATES package_depends)
      list(SORT package_depends)
    endif()
  endwhile()

  set(ALL_INCLUDE_DIRECTORIES)
  set(ALL_LIBRARIES)
  set(ALL_QT4_MODULES ${USE_QT4_MODULES})
  set(ALL_QT5_MODULES ${USE_QT5_MODULES})
  set(ALL_META_DEPENDENCIES)

  foreach(dependency ${depends})
    if(NOT ${dependency}_CONFIG_FILE)
      message(SEND_ERROR "Missing module ${dependency}")
    endif()
    include(${${dependency}_CONFIG_FILE})
    if(${dependency}_IS_DEPRECATED AND NOT MODULE_IS_DEPRECATED)
      # Only print the message if the dependent module
      # is not deprecated itself and if it is a first-level dependency.
      if(first_level_module_deps)
        list(FIND first_level_module_deps ${dependency} _index)
        if(_index GREATER -1)
          message(WARNING "Module ${dependency} is deprecated since ${${dependency}_DEPRECATED_SINCE}")
        endif()
      endif()
    endif()

    list(APPEND ALL_INCLUDE_DIRECTORIES ${${dependency}_INCLUDE_DIRS})
    list(APPEND ALL_LIBRARIES ${${dependency}_PROVIDES})
    list(APPEND ALL_QT4_MODULES ${${dependency}_QT4_MODULES})
    list(APPEND ALL_QT5_MODULES ${${dependency}_QT5_MODULES})
    if(TARGET ${dependency}-autoload)
      list(APPEND ALL_META_DEPENDENCIES ${dependency}-autoload)
    endif()
  endforeach(dependency)

  if(ALL_QT4_MODULES)
    list(REMOVE_DUPLICATES ALL_QT4_MODULES)
  endif()
  if(ALL_QT5_MODULES)
    list(REMOVE_DUPLICATES ALL_QT5_MODULES)
  endif()

  set(MODULE_QT4_MODULES ${ALL_QT4_MODULES})
  foreach(package ${package_depends})
    foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
      if(EXISTS "${dir}/MITK_${package}_Config.cmake")
        include("${dir}/MITK_${package}_Config.cmake")
        break()
      endif()
    endforeach()
  endforeach()

  if(depends)
    list(APPEND ALL_INCLUDE_DIRECTORIES ${MODULES_CONF_DIRS})
  endif()

  if(ALL_INCLUDE_DIRECTORIES)
    list(REMOVE_DUPLICATES ALL_INCLUDE_DIRECTORIES)
    include_directories(${ALL_INCLUDE_DIRECTORIES})
  endif()

  if(ALL_LIBRARIES)
    target_link_libraries(${USE_TARGET} ${ALL_LIBRARIES})
  endif()

  set(ALL_INCLUDE_DIRECTORIES ${ALL_INCLUDE_DIRECTORIES} PARENT_SCOPE)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} PARENT_SCOPE)
  set(ALL_QT4_MODULES ${ALL_QT4_MODULES} PARENT_SCOPE)
  set(ALL_QT5_MODULES ${ALL_QT5_MODULES} PARENT_SCOPE)
  set(ALL_META_DEPENDENCIES ${ALL_META_DEPENDENCIES} PARENT_SCOPE)

endfunction()
