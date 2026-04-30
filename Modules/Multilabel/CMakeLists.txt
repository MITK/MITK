mitk_create_module(
  PCH
    mitkBaseData.h
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase MitkDICOMQI
  PACKAGE_DEPENDS ITK|Smoothing+LabelMap
)

add_subdirectory(autoload/IO)
add_subdirectory(autoload/DICOMSegIO)

if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
