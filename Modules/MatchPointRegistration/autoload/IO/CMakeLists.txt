mitk_create_module(MatchPointRegistrationIO
  DEPENDS
    PUBLIC MitkMatchPointRegistration MitkSceneSerializationBase
  PACKAGE_DEPENDS
    PRIVATE MatchPoint
  AUTOLOAD_WITH MitkCore
)
