MITK_CREATE_MODULE(MitkMapperExt
                   DEPENDS MitkDataTypesExt Shaders
                   PACKAGE_DEPENDS VTK|vtkRenderingVolumeOpenGL
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
