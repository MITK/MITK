###################################################################
#
# MITK_CREATE_MODULE_CONF
#
# This can be called in a similar way like MITK_CREATE_MODULE
# but it just creates the module configuration files without
# actually building it. It is used for integration of legacy libraries
# into the MITK module build system
#
##################################################################
macro(MITK_CREATE_MODULE_CONF MODULE_NAME_IN)
  MACRO_PARSE_ARGUMENTS(MODULE "INCLUDE_DIRS;DEPENDS;" "QT_MODULE;HEADERS_ONLY" ${ARGN})
  set(MODULE_NAME ${MODULE_NAME_IN})
  if(MODULE_HEADERS_ONLY)
    set(MODULE_PROVIDES )
  else()
    set(MODULE_PROVIDES ${MODULE_NAME})
  endif()
  set(MODULE_IS_ENABLED 1)
  _MITK_CREATE_MODULE_CONF()
endmacro(MITK_CREATE_MODULE_CONF)

macro(_MITK_CREATE_MODULE_CONF)
  if(NOT MODULE_QT_MODULE OR MITK_USE_QT)
    set(MODULE_INCLUDE_DIRS_ABSOLUTE "")
    foreach(dir ${MODULE_INCLUDE_DIRS})
      get_filename_component(abs_dir ${dir} ABSOLUTE)
      set(MODULE_INCLUDE_DIRS_ABSOLUTE ${MODULE_INCLUDE_DIRS_ABSOLUTE} ${abs_dir})
    endforeach(dir)

    set(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS_ABSOLUTE} ${CMAKE_CURRENT_SOURCE_DIR})

    # Qt generates headers in the binary tree
    if(MODULE_QT_MODULE)
      set(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    endif(MODULE_QT_MODULE)
    set(${MODULE_NAME}_CONFIG_FILE "${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/${MODULE_NAME}Config.cmake" CACHE INTERNAL "Path to module config" FORCE)
    configure_file(${MITK_SOURCE_DIR}/CMake/moduleConf.cmake.in ${${MODULE_NAME}_CONFIG_FILE} @ONLY)
  endif(NOT MODULE_QT_MODULE OR MITK_USE_QT)
endmacro(_MITK_CREATE_MODULE_CONF)
