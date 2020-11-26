mitk_create_module(
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase MitkDICOMQI
)

add_subdirectory(autoload/IO)
add_subdirectory(autoload/DICOMSegIO)

if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
