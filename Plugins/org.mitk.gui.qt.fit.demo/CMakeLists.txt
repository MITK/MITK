project(org_mitk_gui_qt_fit_demo)

mitk_create_plugin(
    EXPORT_DIRECTIVE FitGeneratorDemo_EXPORT
	EXPORTED_INCLUDE_SUFFIXES src
	MODULE_DEPENDS MitkQtWidgetsExt MitkTestingHelper MitkModelFit MitkModelFitUI
)
