MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt MitkLegacyGL
                   PACKAGE_DEPENDS VTK|vtkRenderingVolumeOpenGL
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
