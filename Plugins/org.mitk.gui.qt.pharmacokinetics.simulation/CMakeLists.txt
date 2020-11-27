project(org_mitk_gui_qt_pharmacokinetics_simulation)

mitk_create_plugin(
        EXPORT_DIRECTIVE PERFUSIONDATASIMULATION_EXPORT
	EXPORTED_INCLUDE_SUFFIXES src
        MODULE_DEPENDS MitkQtWidgetsExt MitkPharmacokinetics
)
