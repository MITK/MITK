project(mitkWorkbench)

set(_app_options)
if(MITK_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

MITK_USE_MODULE(qtsingleapplication Mitk)
include_directories(${ALL_INCLUDE_DIRECTORIES})

# Create a cache entry for the provisioning file which is used to export
# the file name in the MITKConfig.cmake file. This will keep external projects
# which rely on this file happy.
set(MITK_EXTAPP_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitkWorkbench.provisioning" CACHE INTERNAL "mitkWorkbench provisioning file" FORCE)

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
  org.blueberry.test
  org.blueberry.uitest
  org.mitk.gui.qt.coreapplication
  org.mitk.gui.qt.diffusionimagingapp
)

FunctionCreateBlueBerryApplication(
  NAME mitkWorkbench
  DESCRIPTION "MITK Workbench"
  EXCLUDE_PLUGINS ${_exclude_plugins}
  LINK_LIBRARIES ${ALL_LIBRARIES}
  ${_app_options}
)

#Setting application icon for mac os x systems
set_target_properties(mitkWorkbench PROPERTIES MACOSX_BUNDLE_ICON_FILE "icon.icns")

if(APPLE)
  install(FILES "icons/icon.icns" DESTINATION "mitkWorkbench.app/Contents/Resources")
endif(APPLE)

# Add a build time dependency to legacy BlueBerry bundles.
if(MITK_MODULES_ENABLED_PLUGINS)
  add_dependencies(mitkWorkbench ${MITK_MODULES_ENABLED_PLUGINS})
endif()

