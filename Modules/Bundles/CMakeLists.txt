
IF(MITK_USE_BLUEBERRY)

  # INCLUDE("${MITK_BINARY_DIR}/mitkExtConfig.cmake")
  # INCLUDE("${MITK_BINARY_DIR}/QmitkExtConfig.cmake")

  INCLUDE("${MITK_BINARY_DIR}/Bundles/MITKCoreBundleList.cmake")
  
  SET(MITK_DEFAULT_SUBPROJECTS MITK-Plugins)
  
  SET(_mitk_bundles_default 
      "org.mitk.planarfigure"
  )
  
  IF(QT4_FOUND)
    LIST(APPEND _mitk_bundles_default "org.mitk.gui.qt.ext" "org.mitk.gui.qt.extapplication")
    # ExtApp required bundles
    LIST(APPEND _mitk_bundles_default "org.mitk.gui.qt.imagenavigator" "org.mitk.gui.qt.datamanager")

    # BlueBerry depends on CTK which depends on Qt, so we
    # need the Qt include direcotries
    INCLUDE(${QT_USE_FILE})
  ENDIF()
  
  SET(PLUGIN_EXCLUDE_LIST
    org.mitk.core.ext
    org.mitk.gui.qt.volumetry
    org.mitk.core.jobs
    org.mitk.diffusionimaging
    org.mitk.gui.qt.diffusionimaging
    org.mitk.gui.qt.ext
    org.mitk.gui.qt.extapplication
    org.mitk.gui.qt.datamanager
    org.mitk.planarfigure
  )

  MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ExtBundles
                        CACHE_PLUGIN_SOURCE_DIRS MITK_MODULES_PLUGIN_SOURCE_DIRS
                        CACHE_PLUGIN_OUTPUT_DIRS MITK_MODULES_PLUGIN_OUTPUT_DIRS
                        CACHE_PLUGIN_TARGETS MITK_MODULES_ENABLED_PLUGINS
                        BUNDLE_LIST_PATH "${MITK_BINARY_DIR}/Bundles/MITKModulesBundleList.cmake" 
                        CMAKE_CACHE_PREFIX "MITK"
                        PLUGIN_DEFAULT_ON ${_mitk_bundles_default}
                        PLUGIN_EXCLUDES ${PLUGIN_EXCLUDE_LIST}
                        ${MITK_BUILD_ALL_PLUGINS_OPTION})
   
ENDIF(MITK_USE_BLUEBERRY)
