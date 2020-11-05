mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS ACVD VTK|IOPLY+IOMINC
  )

add_subdirectory(Testing)

