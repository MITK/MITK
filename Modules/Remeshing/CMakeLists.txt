mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS ACVD VTK|vtkIOPLY+vtkIOMINC
    WARNINGS_AS_ERRORS
  )

add_subdirectory(Testing)

