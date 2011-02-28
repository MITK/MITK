MITK_CREATE_MODULE(MitkRigidRegistrationUI
  SUBPROJECTS MITK-Registration
  INCLUDE_DIRS RigidRegistrationMetrics RigidRegistrationOptimizer RigidRegistrationTransforms
  DEPENDS Qmitk MitkRigidRegistration
  QT_MODULE
  PROVIDES MitkRigidRegistrationUI
  EXPORT_DEFINE MITK_RIGIDREGISTRATION_UI_EXPORT
)
