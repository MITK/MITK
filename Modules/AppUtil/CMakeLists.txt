set(qt5_depends Qt5|Widgets)

if(UNIX AND NOT APPLE)
  set(qt5_depends "${qt5_depends}+X11Extras")
endif()

mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework ${qt5_depends} Poco|Util
    PRIVATE VTK
  DEPENDS
    PUBLIC qtsingleapplication
    PRIVATE MitkCore
)
