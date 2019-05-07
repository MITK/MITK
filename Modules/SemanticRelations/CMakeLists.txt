MITK_CREATE_MODULE(
  DEPENDS MitkSceneSerializationBase MitkDICOMReader MitkMultilabel MitkPersistence MitkImageStatisticsUI
)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Test)
endif(BUILD_TESTING)
