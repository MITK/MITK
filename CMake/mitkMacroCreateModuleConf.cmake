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
  MACRO_PARSE_ARGUMENTS(MODULE "INCLUDE_DIRS;DEPENDS;PACKAGE_DEPENDS;QT4_MODULES;QT5_MODULES" "QT_MODULE;HEADERS_ONLY" ${ARGN})
  set(MODULE_NAME ${MODULE_NAME_IN})
  if(MODULE_HEADERS_ONLY)
    set(MODULE_PROVIDES )
  else()
    set(MODULE_PROVIDES ${MODULE_NAME})
  endif()
  set(MODULE_IS_ENABLED 1)

  if(MODULE_QT_MODULE)
    message(WARNING "The QT_MODULE option is deprecated. Use QT4_MODULES and/or QT5_MODULES instead.")
    if(NOT MODULE_QT4_MODULES)
      set(MODULE_QT4_MODULES QtGui)
    endif()
  endif()

  _MITK_CREATE_MODULE_CONF()
endmacro(MITK_CREATE_MODULE_CONF)

macro(_MITK_CREATE_MODULE_CONF)
  set(${MODULE_NAME}_CONFIG_FILE "${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/${MODULE_NAME}Config.cmake" CACHE INTERNAL "Path to module config" FORCE)
  configure_file(${MITK_SOURCE_DIR}/CMake/moduleConf.cmake.in ${${MODULE_NAME}_CONFIG_FILE} @ONLY)
endmacro(_MITK_CREATE_MODULE_CONF)
