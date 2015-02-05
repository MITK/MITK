MITK_CREATE_MODULE(
  SUBPROJECTS MITK-Registration
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC ITK|ITKRegistrationCommon
)

add_subdirectory(Testing)
