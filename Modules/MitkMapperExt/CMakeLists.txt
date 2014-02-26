MITK_CREATE_MODULE(MitkMapperExt
                   DEPENDS MitkDataTypesExt VtkShaders
                   PACKAGE_DEPENDS VTK|vtkRenderingVolumeOpenGL
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
