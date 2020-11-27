mitk_create_module(
  DEPENDS MitkDataTypesExt MitkLegacyGL
  PACKAGE_DEPENDS
    PUBLIC OpenMP
    PRIVATE ANN VTK|ImagingGeneral
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
