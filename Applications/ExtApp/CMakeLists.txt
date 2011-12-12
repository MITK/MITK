project(ExtApp)

set(_app_options)
if(MITK_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
  org.blueberry.test
  org.blueberry.uitest
  org.mitk.gui.qt.application
  org.mitk.gui.qt.diffusionimagingapp
)

FunctionCreateBlueBerryApplication(
  NAME ExtApp
  DESCRIPTION "MITK - ExtApp Application"
  EXCLUDE_PLUGINS ${_exclude_plugins}
  ${_app_options}
)

# Add a build time dependency to legacy BlueBerry bundles.
if(MITK_MODULES_ENABLED_PLUGINS)
  add_dependencies(ExtApp ${MITK_MODULES_ENABLED_PLUGINS})
endif()

# Create a cache entry for the provisioning file which is used to export
# the file name in the MITKConfig.cmake file. This will keep external projects
# which rely on this file happy.
set(MITK_EXTAPP_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ExtApp.provisioning" CACHE INTERNAL "ExtApp provisioning file" FORCE)
