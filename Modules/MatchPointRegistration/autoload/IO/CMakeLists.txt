MITK_CREATE_MODULE(MatchPointRegistrationIO
  DEPENDS
    PUBLIC MitkMatchPointRegistration MitkSceneSerializationBase
  PACKAGE_DEPENDS
    PRIVATE ITK MatchPoint
  AUTOLOAD_WITH MitkCore
)
