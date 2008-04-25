# plugin dependencies
SET(PLUGIN_REQUIRED_LIBS 
  org_opencherry_ui_qt 
  org_opencherry_application
  ${QT_LIBRARIES}
)

ADD_DEFINITIONS(${QT_DEFINITIONS})

CREATE_PLUGIN(org.opencherry.application.qtworkbench)