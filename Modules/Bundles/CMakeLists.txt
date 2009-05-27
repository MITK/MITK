
IF(MITK_USE_OPENCHERRY)

  INCLUDE("${MITK_BINARY_DIR}/mitkExtConfig.cmake")
  INCLUDE("${MITK_BINARY_DIR}/QmitkExtConfig.cmake")

  INCLUDE("${MITK_BINARY_DIR}/Bundles/MITKCoreBundleList.cmake")
  
  SET(_mitk_bundles_default "org.mitk.core.ext")
  IF(QT4_FOUND)
    LIST(APPEND _mitk_bundles_default "org.mitk.gui.qt.ext")
  ENDIF()

  MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ExtBundles
                        CACHE_PLUGIN_SOURCE_DIRS MITK_MODULES_PLUGIN_SOURCE_DIRS
                        CACHE_PLUGIN_OUTPUT_DIRS MITK_MODULES_PLUGIN_OUTPUT_DIRS
                        CACHE_PLUGIN_TARGETS MITK_MODULES_ENABLED_PLUGINS
                        BUNDLE_LIST_PATH "${MITK_BINARY_DIR}/Bundles/MITKModulesBundleList.cmake" 
                        CMAKE_CACHE_PREFIX "MITK"
                        PLUGIN_DEFAULT_ON ${_mitk_bundles_default}
                        ${MITK_BUILD_ALL_PLUGINS_OPTION})
   
ENDIF(MITK_USE_OPENCHERRY)
