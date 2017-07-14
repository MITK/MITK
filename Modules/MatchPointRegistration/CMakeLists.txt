MITK_CREATE_MODULE(
  INCLUDE_DIRS PUBLIC Rendering Helper
  DEPENDS MitkCore MitkSceneSerializationBase
  PACKAGE_DEPENDS
   PUBLIC MatchPoint
)

if( ${MITK_USE_MatchPoint} )
  ADD_SUBDIRECTORY(autoload/IO)
  ADD_SUBDIRECTORY(algorithms)
  if(BUILD_TESTING)
    ADD_SUBDIRECTORY(Testing)
  endif(BUILD_TESTING)
endif()
