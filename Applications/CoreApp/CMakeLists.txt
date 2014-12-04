project(CoreApp)

set(_app_options)
if(MITK_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

# Create a cache entry for the provisioning file which is used to export
# the file name in the MITKConfig.cmake file. This will keep external projects
# which rely on this file happy.
set(MITK_COREAPP_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MitkCoreApp.provisioning" CACHE INTERNAL "CoreApp provisioning file" FORCE)

set(_plugins
  org.blueberry.compat
  org.mitk.gui.qt.coreapplication
  org.mitk.gui.qt.stdmultiwidgeteditor
)

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins )

FunctionCreateBlueBerryApplication(
  NAME MitkCoreApp
  DESCRIPTION "MITK - CoreApp Application"
  PLUGINS ${_plugins}
  ${_app_options}
)
mitk_use_modules(TARGET MitkCoreApp PACKAGES Qt4|QtGui Qt5|Widgets)

# subproject support
add_dependencies(MITK-CoreUI MitkCoreApp)
