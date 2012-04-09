MITK_CREATE_MODULE(MitkDeformableRegistration
  SUBPROJECTS MITK-Registration
  DEPENDS Mitk MitkRigidRegistration
  EXPORT_DEFINE MITK_DEFORMABLEREGISTRATION_EXPORT
)

add_subdirectory(Testing)
