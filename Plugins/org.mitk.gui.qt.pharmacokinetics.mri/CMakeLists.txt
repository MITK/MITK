project(org_mitk_gui_qt_pharmacokinetics_mri)

mitk_create_plugin(
    EXPORT_DIRECTIVE MRPERFUSION_EXPORT
	EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDS MitkQtWidgetsExt MitkPharmacokinetics MitkModelFitUI
)
