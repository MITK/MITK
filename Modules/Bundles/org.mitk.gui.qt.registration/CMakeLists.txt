# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
PROJECT(org_mitk_gui_qt_registration)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE REGISTRATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt MitkDeformableRegistration MitkDeformableRegistrationUI MitkRigidRegistration MitkRigidRegistrationUI
)

