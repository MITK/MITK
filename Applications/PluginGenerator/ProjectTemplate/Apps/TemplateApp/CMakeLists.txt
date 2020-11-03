
# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins

)

mitkFunctionCreateBlueBerryApplication(
  NAME ${MY_APP_NAME}
  DESCRIPTION "MITK - ${MY_APP_NAME} Application"
  EXCLUDE_PLUGINS ${_exclude_plugins}
)
