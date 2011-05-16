
IF(MITK_USE_BLUEBERRY)

  # Those are CTK plugins
  SET(_exclude_plugins
    org.mitk.core.services
  )

  INCLUDE(${QT_USE_FILE})

  MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreBundles
                        CACHE_PLUGIN_SOURCE_DIRS MITK_CORE_PLUGIN_SOURCE_DIRS
                        CACHE_PLUGIN_OUTPUT_DIRS MITK_CORE_PLUGIN_OUTPUT_DIRS
                        CACHE_PLUGIN_TARGETS MITK_CORE_ENABLED_PLUGINS
                        BUNDLE_LIST_PATH "${MITK_BINARY_DIR}/Bundles/MITKCoreBundleList.cmake"
                        CMAKE_CACHE_PREFIX "MITK"
#                        DEPENDS ${BLUEBERRY_PLUGINS_OUTPUT_DIR}
                        DEFAULT_BUILD_ON
                        PLUGIN_EXCLUDES ${_exclude_plugins}
                        ${MITK_BUILD_ALL_PLUGINS_OPTION})
   
ENDIF(MITK_USE_BLUEBERRY)
