MITK_CREATE_MODULE(MitkDeformableRegistrationUI
  SUBPROJECTS MITK-Registration
  DEPENDS Qmitk MitkDeformableRegistration
  QT_MODULE
  PROVIDES MitkDeformableRegistrationUI
  EXPORT_DEFINE MITK_DEFORMABLEREGISTRATION_UI_EXPORT
)
