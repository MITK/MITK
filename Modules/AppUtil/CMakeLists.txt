mitk_create_module(PACKAGE_DEPENDS
                     PUBLIC CTK|CTKPluginFramework Qt4|QtGui Qt5|Widgets Poco|Util
                   DEPENDS
                     PUBLIC qtsingleapplication
                     PRIVATE MitkCore
                   WARNINGS_AS_ERRORS
                  )

