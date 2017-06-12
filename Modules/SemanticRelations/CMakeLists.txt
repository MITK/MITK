MITK_CREATE_MODULE(
  DEPENDS MitkSceneSerializationBase MitkMultilabel MitkPersistence
  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
