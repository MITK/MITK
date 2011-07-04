
set(BLUEBERRY_USE_QT ${MITK_USE_QT} CACHE BOOL "Use the Qt GUI toolkit" FORCE)
mark_as_advanced(BLUEBERRY_USE_QT)
    
# Clear the cache variables containing plugin base directories
set(MITK_MODULES_PLUGIN_SOURCE_DIRS "" CACHE INTERNAL "List of base plugin source directories" FORCE)
set(MITK_MODULES_PLUGIN_OUTPUT_DIRS "" CACHE INTERNAL "List of base plugin output directories" FORCE)

set(mbilog_DIR "${mbilog_BINARY_DIR}")

if(MITK_BUILD_ALL_PLUGINS)
  set(BLUEBERRY_BUILD_ALL_PLUGINS ON)
endif()

add_subdirectory(BlueBerry)

set(BlueBerry_DIR ${CMAKE_CURRENT_BINARY_DIR}/BlueBerry
    CACHE PATH "The directory containing a CMake configuration file for BlueBerry" FORCE)
include(${BlueBerry_DIR}/BlueBerryConfig.cmake)
include(mitkMacroCreatePlugin)
include(mitkMacroCreateCTKPlugin)

SET(CTK_PLUGIN_LIBRARIES ${CTK_PLUGIN_LIBRARIES} CACHE INTERNAL "CTK plugins" FORCE)

# Extract all library names starting with org_mitk_
MACRO(GetMyTargetLibraries all_target_libraries varname)
  SET(re_ctkplugin "^org_mitk_[a-zA-Z0-9_]+$")
  SET(_tmp_list)
  LIST(APPEND _tmp_list ${all_target_libraries})
  ctkMacroListFilter(_tmp_list re_ctkplugin OUTPUT_VARIABLE ${varname})
ENDMACRO()
