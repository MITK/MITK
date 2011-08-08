
#
# Convenient macro allowing to define a "empty" project in case an external one is provided
# using for example <proj>_DIR. 
# Doing so allows to keep the external project dependency system happy.
#
macro(MacroEmptyExternalProject proj dependencies)

  ExternalProject_Add(${proj}
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS 
      ${dependencies}
    )
    
endmacro()
