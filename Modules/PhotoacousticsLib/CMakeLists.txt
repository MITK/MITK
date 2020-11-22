MITK_CREATE_MODULE(
  INCLUDE_DIRS PUBLIC include
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS PUBLIC MitkAlgorithmsExt
  PACKAGE_DEPENDS
    tinyxml2|tinyxml2 Vigra
    PUBLIC ITK
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PRIVATE tinyxml2::tinyxml2
  )
endif()

add_subdirectory(MitkMCxyz)

add_subdirectory(MitkTissueBatchGenerator)
add_subdirectory(MitkPAPhantomGenerator)
add_subdirectory(MitkSpectralUnmixing)

add_subdirectory(test)
