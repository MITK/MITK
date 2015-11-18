project(Qmlbench)

set(PROJECT_NAME Qmlbench)
set(APP_NAME MitkQmlbench)

set(_app_options)
if(${PROJECT_NAME}_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins

)

mitkFunctionCreateBlueBerryApplication(
  NAME ${APP_NAME}
  DESCRIPTION "MITK - ${APP_NAME} Application"
  EXCLUDE_PLUGINS ${_exclude_plugins}
  ${_app_options}
)
mitk_use_modules(TARGET ${APP_NAME}
  MODULES MitkAppUtil MitkQmlItems
  PACKAGES Qt5|Core+Quick
)