
SET(_default_plugins
  org.blueberry.osgi
  org.blueberry.solstice.common
  org.blueberry.core.runtime
  org.blueberry.core.expressions
  org.blueberry.core.commands
  org.blueberry.ui
  org.blueberry.ui.qt
)

MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${OPENCHERRY_PLUGINS_OUTPUT_DIR}
                      CACHE_PLUGIN_TARGETS OPENCHERRY_ENABLED_PLUGINS
                      BUNDLE_LIST_PATH "${OPENCHERRY_BINARY_DIR}/openCherryBundleList.cmake"
                      CMAKE_CACHE_PREFIX "OPENCHERRY"
                      PLUGIN_DEFAULT_ON ${_default_plugins}
                      ${OPENCHERRY_BUILD_ALL_PLUGINS_OPTION})
					  