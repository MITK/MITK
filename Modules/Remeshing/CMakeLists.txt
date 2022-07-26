mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS ACVD|Surface+VolumeProcessing VTK|IOPLY+IOMINC
)

add_subdirectory(Testing)
