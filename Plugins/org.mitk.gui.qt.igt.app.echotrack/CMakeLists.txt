project(org_mitk_gui_qt_igtappechotrack)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE IGTAPPECHOTRACK_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  PACKAGE_DEPENDS CTK Poco
  MODULE_DEPENDS MitkUSUI MitkUSNavigation MitkIGTUI MitkSceneSerialization MitkContourModel
)

#usFunctionAddResources(TARGET ${PLUGIN_TARGET}
#  MODULE_NAME liborg_mitk_gui_qt_usnavigation
#  WORKING_DIRECTORY resources
#  FILES Interactions/USPointMarkInteractions.xml
#        Interactions/USZoneInteractions.xml
#        Interactions/USZoneInteractionsHold.xml
#)
