if(MITK_USE_ACVD)
  MITK_CREATE_MODULE(Remeshing
    DEPENDS Mitk
    PACKAGE_DEPENDS ACVD VTK>vtkIOPLY|vtkIOMINC
  )

  add_subdirectory(Testing)
endif()
