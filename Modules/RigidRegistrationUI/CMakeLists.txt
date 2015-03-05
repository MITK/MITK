MITK_CREATE_MODULE(
  SUBPROJECTS MITK-Registration
  INCLUDE_DIRS RigidRegistrationMetrics RigidRegistrationOptimizer RigidRegistrationTransforms
  DEPENDS MitkQtWidgets MitkRigidRegistration
)
