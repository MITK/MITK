project(org_mitk_gui_qt_pharmacokinetics_curvedescriptor)

mitk_create_plugin(
    EXPORT_DIRECTIVE CURVEDESCRIPTORS_EXPORT
    EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDS
	    PRIVATE MitkQtWidgetsExt MitkPharmacokinetics MitkModelFitUI MitkPharmacokineticsUI
)
