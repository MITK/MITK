MITK_CREATE_MODULE(
  INCLUDE_DIRS PUBLIC include
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS PUBLIC MitkAlgorithmsExt tinyxml
  PACKAGE_DEPENDS
    tinyxml Vigra
    PUBLIC ITK
)

add_subdirectory(MitkMCxyz)

add_subdirectory(MitkTissueBatchGenerator)
add_subdirectory(MitkPAPhantomGenerator)
add_subdirectory(MitkSpectralUnmixing)

add_subdirectory(test)
