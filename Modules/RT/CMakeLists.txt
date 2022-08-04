mitk_create_module(
  DEPENDS MitkSceneSerializationBase MitkDICOM
  PACKAGE_DEPENDS PRIVATE DCMTK|dcmrt
)

add_subdirectory(autoload/DICOMRTIO)
add_subdirectory(test)
