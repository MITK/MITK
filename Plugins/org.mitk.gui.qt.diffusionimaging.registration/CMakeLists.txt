# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_diffusionimaging_registration)

mitk_create_plugin(
  SUBPROJECTS MITK-DTI
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_REGISTRATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkDiffusionCore MitkMatchPointRegistration MitkMatchPointRegistrationUI
)
