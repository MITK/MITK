
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

include(mitkMacroCreatePlugin)
include(mitkMacroCreateCTKPlugin)
