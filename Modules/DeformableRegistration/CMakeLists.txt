MITK_CREATE_MODULE(
  SUBPROJECTS MITK-Registration
  DEPENDS MitkRigidRegistration
  PACKAGE_DEPENDS PUBLIC ITK|ITKPDEDeformableRegistration
)

add_subdirectory(Testing)
