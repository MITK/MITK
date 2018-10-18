MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkMatchPointRegistration MitkMatchPointRegistrationIO
  PACKAGE_DEPENDS
    PUBLIC
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM
)

add_subdirectory(MiniApps)
