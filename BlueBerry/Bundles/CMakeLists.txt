
SET(_default_plugins
  org.blueberry.osgi
  org.blueberry.solstice.common
  org.blueberry.core.runtime
  org.blueberry.core.expressions
  org.blueberry.core.commands
  org.blueberry.ui
  org.blueberry.ui.qt
)

MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${BLUEBERRY_PLUGINS_OUTPUT_DIR}
                      CACHE_PLUGIN_TARGETS BLUEBERRY_ENABLED_PLUGINS
                      BUNDLE_LIST_PATH "${BLUEBERRY_BINARY_DIR}/BlueBerryBundleList.cmake"
                      CMAKE_CACHE_PREFIX "BLUEBERRY"
                      PLUGIN_DEFAULT_ON ${_default_plugins}
                      ${BLUEBERRY_BUILD_ALL_PLUGINS_OPTION})
					  