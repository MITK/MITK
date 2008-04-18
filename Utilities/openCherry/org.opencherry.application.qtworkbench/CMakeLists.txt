# plugin dependencies
SET(PLUGIN_REQUIRED_LIBS org.opencherry.ui.qt org.opencherry.application)

ADD_DEFINITIONS(${QT_DEFINITIONS})

CREATE_PLUGIN(org.opencherry.application.qtworkbench)

TARGET_LINK_LIBRARIES(org.opencherry.application.qtworkbench ${QT_LIBRARIES})