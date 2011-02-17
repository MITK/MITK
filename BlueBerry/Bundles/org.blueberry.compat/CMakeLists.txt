PROJECT(org_blueberry_compat)

SET(PLUGIN_export_directive "org_blueberry_compat_EXPORT")

SET(PLUGIN_SRCS
  berryCompatibilityActivator.cpp
  berryCTKPluginListener.cpp
)

# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
  berryCompatibilityActivator_p.h
  berryCTKPluginListener_p.h
)

# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
SET(PLUGIN_resources
)

# Remove the following line after the src dir has been removed
INCLUDE_DIRECTORIES(${org_blueberry_osgi_SOURCE_DIR}/src)

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries}
)
