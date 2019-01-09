MITK_CREATE_MODULE(
  DEPENDS MitkSceneSerializationBase MitkDICOMReader MitkMultilabel MitkPersistence
)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
