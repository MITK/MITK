project(org_mitk_gui_qt_igt_app_ultrasoundtrackingnavigation)

mitk_create_plugin(
  EXPORT_DIRECTIVE IGTAPPUSTRACKINGNAVIGATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS PRIVATE CTK ITK|LabelMap Poco
  MODULE_DEPENDS MitkUSUI MitkUSNavigation MitkIGTUI MitkSceneSerialization MitkContourModel
)

#usFunctionAddResources(TARGET ${PLUGIN_TARGET}
#  MODULE_NAME liborg_mitk_gui_qt_usnavigation
#  WORKING_DIRECTORY resources
#  FILES Interactions/USPointMarkInteractions.xml
#        Interactions/USZoneInteractions.xml
#        Interactions/USZoneInteractionsHold.xml
#)
