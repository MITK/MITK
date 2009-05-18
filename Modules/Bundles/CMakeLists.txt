
IF(MITK_USE_OPENCHERRY)

  INCLUDE("${MITK_BINARY_DIR}/mitkExtConfig.cmake")
  INCLUDE("${MITK_BINARY_DIR}/QmitkExtConfig.cmake")

  INCLUDE("${MITK_BINARY_DIR}/Bundles/MITKCoreBundleList.cmake")
  
  SET(_mitk_bundles_default "org.mitk.core.ext")
  IF(QT4_FOUND)
    LIST(APPEND _mitk_bundles_default "org.mitk.gui.qt.ext")
  ENDIF()

  MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${LIBRARY_OUTPUT_PATH}/ExtBundles
                        CACHE_PLUGIN_SOURCE_DIRS MITK_MODULES_PLUGIN_SOURCE_DIRS
                        CACHE_PLUGIN_BINARY_DIRS MITK_MODULES_PLUGIN_BINARY_DIRS
                        CACHE_PLUGIN_TARGETS MITK_MODULES_ENABLED_PLUGINS
                        BUNDLE_LIST_PATH "${MITK_BINARY_DIR}/Bundles/MITKModulesBundleList.cmake" 
                        CMAKE_CACHE_PREFIX "MITK"
                        BUNDLE_DEFAULT_ON ${_mitk_bundles_default}
                        ${MITK_BUILD_ALL_PLUGINS_OPTION})
   
ENDIF(MITK_USE_OPENCHERRY)
