SET(_ctk_plugins
  org.blueberry.osgi:ON
)

SET(_ctk_plugins_include_dirs
  ${Poco_INCLUDE_DIRS}
)

SET(_ctk_plugins_link_dirs
  ${Poco_LIBRARY_DIR}
)

INCLUDE_DIRECTORIES(${_ctk_plugins_include_dirs})
LINK_DIRECTORIES(${_ctk_plugins_link_dirs})

ctkMacroSetupExternalPlugins(${_ctk_plugins} BUILD_OPTION_PREFIX BLUEBERRY_BUILD_)

# Needed for BlueBerry plugins to find CTK plugins
LINK_DIRECTORIES(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins)

SET(_default_plugins
  org.blueberry.solstice.common
  org.blueberry.core.runtime
  org.blueberry.core.expressions
  org.blueberry.core.commands
  org.blueberry.ui
  org.blueberry.ui.qt
)

SET(_exclude_plugins
  org.blueberry.osgi
)

MACRO_COLLECT_PLUGINS(OUTPUT_DIR ${BLUEBERRY_PLUGINS_OUTPUT_DIR}
                      CACHE_PLUGIN_TARGETS BLUEBERRY_ENABLED_PLUGINS
                      BUNDLE_LIST_PATH "${BLUEBERRY_BINARY_DIR}/BlueBerryBundleList.cmake"
                      CMAKE_CACHE_PREFIX "BLUEBERRY"
                      PLUGIN_DEFAULT_ON ${_default_plugins}
                      PLUGIN_EXCLUDES ${_exclude_plugins}
                      ${BLUEBERRY_BUILD_ALL_PLUGINS_OPTION})
            
add_dependencies(BlueBerry ${BLUEBERRY_ENABLED_PLUGINS})
foreach(plugin ${BLUEBERRY_ENABLED_PLUGINS})
  string(REPLACE "." "_" plugin_target ${plugin})
  set_property(TARGET ${plugin_target} PROPERTY LABELS BlueBerry)
endforeach()
