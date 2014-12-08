mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS ACVD VTK|vtkIOPLY+vtkIOMINC
  )

add_subdirectory(Testing)

