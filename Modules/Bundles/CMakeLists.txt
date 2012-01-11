
IF(MITK_USE_BLUEBERRY)

  SET(MITK_DEFAULT_SUBPROJECTS MITK-Plugins)

  SET(_mitk_bundles_default

  )

  IF(MITK_USE_Python)
    LIST(APPEND _mitk_bundles_default org.mitk.gui.qt.python.console)
  ENDIF()
  
  IF(QT4_FOUND)
    LIST(APPEND _mitk_bundles_default "org.mitk.gui.qt.imagenavigator")

    # BlueBerry depends on CTK which depends on Qt, so we
    # need the Qt include direcotries
    INCLUDE(${QT_USE_FILE})
  ENDIF()

  SET(PLUGIN_EXCLUDE_LIST
    org.mitk.core.ext
    org.mitk.core.jobs
    org.mitk.diffusionimaging
    org.mitk.gui.qt.diffusionimaging
    org.mitk.gui.qt.dtiatlasapp
    org.mitk.gui.qt.diffusionimagingapp
    org.mitk.gui.qt.examples
	org.mitk.gui.qt.examplesopencv
    org.mitk.gui.qt.ext
    org.mitk.gui.qt.extapplication
    org.mitk.gui.qt.datamanager
    org.mitk.gui.qt.segmentation
    org.mitk.planarfigure
    org.mitk.gui.qt.igttracking
    org.mitk.gui.qt.igtnavigation
    org.mitk.gui.qt.igtexamples
    org.mitk.gui.qt.basicimageprocessing
    org.mitk.gui.qt.navigationdataplayer
 #  bug-8977 Deactivating ToF for testing purposes
 #   org.mitk.gui.qt.tofutil
 #   org.mitk.gui.qt.toftutorial
    org.mitk.gui.qt.imagenavigator
    org.mitk.gui.qt.moviemaker
    org.mitk.gui.qt.materialeditor
    org.mitk.gui.qt.ugvisualization
    org.mitk.gui.qt.measurementtoolbox
    org.mitk.gui.qt.imagecropper
    org.mitk.gui.qt.volumevisualization
    org.mitk.gui.qt.pointsetinteraction
    org.mitk.gui.qt.python.console
	org.mitk.gui.qt.registration
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
