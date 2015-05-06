if(UNIX AND DESIRED_QT_VERSION MATCHES 4)
  set( CMAKE_CXX_FLAGS  "-fPIC ${CMAKE_CXX_FLAGS}" )
endif()

mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC Qt4|QtGui Qt5|Widgets
    PRIVATE Qt4|QtNetwork Qt5|Network
  FORCE_STATIC
  NO_INIT
)
